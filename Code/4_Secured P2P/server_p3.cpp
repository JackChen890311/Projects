#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#define LEN 32
#define MAX_THREADS 64
#define MAX_QUEUE 65536
#define THREAD 32 // A pool can handle 32 online users
#define QUEUE 256 // A pool can have 256 users waiting

using namespace std;

SSL_CTX* InitServerCTX()
{
    SSL_CTX *ctx;
    /* SSL 庫初始化 */
    SSL_library_init();
    /* 載入所有 SSL 演算法 */
    OpenSSL_add_all_algorithms();
    /* 載入所有 SSL 錯誤訊息 */
    SSL_load_error_strings();
    /* 以 SSL V2 和 V3 標準相容方式產生一個 SSL_CTX ，即 SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_server_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 單獨表示 V2 或 V3標準 */
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stdout);
        abort();
    }
    return ctx;
}

void DynamicCertificates(SSL_CTX* ctx)
{
	EVP_PKEY * pkey;
	pkey = EVP_PKEY_new();

	RSA * rsa;
	rsa = RSA_generate_key(
	    2048,   /* number of bits for the key - 2048 is a sensible value */
	    RSA_F4, /* exponent - RSA_F4 is defined as 0x10001L */
	    NULL,   /* callback - can be NULL if we aren't displaying progress */
	    NULL    /* callback argument - not needed in this case */);
	if (rsa == NULL)
	{
		printf("RSA Failed!\n");
		return;
	}
	EVP_PKEY_assign_RSA(pkey, rsa);
	
	X509 * x509;
	x509 = X509_new();
	ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

	X509_gmtime_adj(X509_get_notBefore(x509), 0);
	X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);
	
	X509_set_pubkey(x509, pkey);
	
	X509_NAME * name;
	name = X509_get_subject_name(x509);
	
	X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC,
                           (unsigned char *)"TW", -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC,
                           (unsigned char *)"NTU", -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                           (unsigned char *)"localhost", -1, -1, 0);
                           
	X509_set_issuer_name(x509, name);
	X509_sign(x509, pkey, EVP_sha1());

	/* 載入使用者的數字證書， 此證書用來發送給客戶端。 證書裡包含有公鑰 */
	if ( SSL_CTX_use_certificate(ctx, x509) <= 0 )
	{
		ERR_print_errors_fp(stderr);
		abort();
	}
	/* 載入使用者私鑰 */
	if ( SSL_CTX_use_PrivateKey(ctx, pkey) <= 0 )
	{
		ERR_print_errors_fp(stderr);
		abort();
	}
	/* 檢查使用者私鑰是否正確 */
	if ( !SSL_CTX_check_private_key(ctx) )
	{
		fprintf(stderr, "Private key does not match the public certificate\n");
		abort();
	}
}


void ShowCerts(SSL *ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL)
    {
        printf("Digital certificate information:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Certificate: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificate information！\n");
}

//==================================================
typedef struct threadpool_t threadpool_t;

typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
} threadpool_error_t;

typedef enum {
    threadpool_graceful       = 1
} threadpool_destroy_flags_t;

/* 以下是Thread Pool三個對外 API */

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags);

int threadpool_add(threadpool_t *pool, void (*routine)(void *),
                   void *arg, int flags);

int threadpool_destroy(threadpool_t *pool, int flags);
//==================================================

typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} threadpool_shutdown_t;

typedef struct {
    void (*function)(void *);
    void *argument;
} threadpool_task_t;

struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  threadpool_task_t *queue;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
};

static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t *pool;
    int i;
    (void) flags;

    if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
        return NULL;
    }

    if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        goto err;
    }

    /* Initialize */
    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;

    /* Allocate thread and task queue */
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (threadpool_task_t *)malloc
        (sizeof(threadpool_task_t) * queue_size);

    /* Initialize mutex and conditional variable first */
    if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
       (pthread_cond_init(&(pool->notify), NULL) != 0) ||
       (pool->threads == NULL) ||
       (pool->queue == NULL)) {
        goto err;
    }

    /* Start worker threads */
    for(i = 0; i < thread_count; i++) {
        if(pthread_create(&(pool->threads[i]), NULL,
                          threadpool_thread, (void*)pool) != 0) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        pool->thread_count++;
        pool->started++;
    }

    return pool;

 err:
    if(pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_add(threadpool_t *pool, void (*function)(void *),
                   void *argument, int flags)
{
    int err = 0;
    int next;
    (void) flags;

    if(pool == NULL || function == NULL) {
        return threadpool_invalid;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    next = (pool->tail + 1) % pool->queue_size;

    do {
        /* Are we full ? */
        if(pool->count == pool->queue_size) {
            err = threadpool_queue_full;
            break;
        }

        /* Are we shutting down ? */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Add task to queue */
        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;

        /* pthread_cond_broadcast */
        if(pthread_cond_signal(&(pool->notify)) != 0) {
            err = threadpool_lock_failure;
            break;
        }
    } while(0);

    if(pthread_mutex_unlock(&pool->lock) != 0) {
        err = threadpool_lock_failure;
    }

    return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
    int i, err = 0;

    if(pool == NULL) {
        return threadpool_invalid;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    do {
        /* Already shutting down */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        pool->shutdown = (flags & threadpool_graceful) ?
            graceful_shutdown : immediate_shutdown;

        /* Wake up all worker threads */
        if((pthread_cond_broadcast(&(pool->notify)) != 0) ||
           (pthread_mutex_unlock(&(pool->lock)) != 0)) {
            err = threadpool_lock_failure;
            break;
        }

        /* Join all worker thread */
        for(i = 0; i < pool->thread_count; i++) {
            if(pthread_join(pool->threads[i], NULL) != 0) {
                err = threadpool_thread_failure;
            }
        }
    } while(0);

    /* Only if everything went well do we deallocate the pool */
    if(!err) {
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL || pool->started > 0) {
        return -1;
    }

    /* Did we manage to allocate ? */
    if(pool->threads) {
        free(pool->threads);
        free(pool->queue);
 
        /* Because we allocate pool->threads after initializing the
           mutex and condition variable, we're sure they're
           initialized. Let's lock the mutex just in case. */
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
    }
    free(pool);    
    return 0;
}


static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    for(;;) {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        while((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if((pool->shutdown == immediate_shutdown) ||
           ((pool->shutdown == graceful_shutdown) &&
            (pool->count == 0))) {
            break;
        }

        /* Grab our task */
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        /* Unlock */
        pthread_mutex_unlock(&(pool->lock));

        /* Get to work */
        (*(task.function))(task.argument);
    }

    pool->started--;

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return(NULL);
}

//==================================================
int clientcnt = 0;
pthread_mutex_t lock;

//==================================================
//class
class User
{
	public:
		string name;
		int money;
		int status;
		string IP;
		int portnum;
		User();
		User(string name, int money);
		void userinfo();
		string getuserinfo();
		void setonline(string ipaddr, int logport);
		void setoffline();	
		void transfer(User *user,int money);
		
};

User::User()
{
	this->name = "NA";
	this->money = -1;
	this->status = 0;
	this->IP = "NA";
	this->portnum = -1;
}

User::User(string name, int money)
{
	this->name = name;
	this->money = money;
	this->status = 0;
	this->IP = "NA";
	this->portnum = -1;
}

void User::userinfo()
{
	cout << "User information:\n";
	cout << "  User Name:" << this->name << "\n";
	cout << "  User Money:" << this->money << "\n";
	cout << "  User Status:" << this->status << "\n";
	cout << "  User IP address:" << this->IP << "\n";
	cout << "  User Port number:" << this->portnum << "\n";
	cout << "===== End =====\n";
}

string User::getuserinfo()
{
	string info;
	info = "User information:\n  User Name:"; 
	info.append(this->name);
	info.append("\n  User Money:");
	info.append(to_string(this->money));
	info.append("\n  User Status:");
	info.append(to_string(this->status));
	info.append("\n  User IP address:");
	info.append(this->IP);
	info.append("\n  User Port number:");
	info.append(to_string(this->portnum));
	info.append("\n===== End =====\n");
	return info;
}


void User::setonline(string ipaddr,int logport)
{	
	this->status = 1;
	this->IP = ipaddr;
	this->portnum = logport;
}

void User::setoffline()
{	
	this->status = 0;
	this->IP = "NA";
	this->portnum = -1;
}

void User::transfer(User *user,int money)
{	
	this->money -= money;
	user->money += money;
}


class List
{
	public:
		int cnt;
		vector<User> reg;
		List();
		void listinfo();
		int finduser(string name);
		string getonlinelist();
};

List::List()
{
	this->cnt = 0;
	//cout <<"List Ready!\n";
}

void List::listinfo()
{
	cout << "List information:\n";
	cout << "List cnt:" << this->cnt << "\n";
	for(int i = 0;i<this->cnt;i++)
	{	
		cout << "----- " << i << "-----\n";
		this->reg[i].userinfo();
	}
	cout << "----- List End -----\n";
}

int List::finduser(string name)
{	
	for(int i = 0;i<this->cnt;i++)
	{	
		//cout << "regname ="<< reg[i].name << "=\n";
		//cout << "name ="<< name << "=\n";
		if(this->reg[i].name.compare(name) == 0)
		{
			return i;
		}
	}
	return -1;
	

}

string List::getonlinelist()
{
	string list;
	int oncnt = 0;
	for(int i = 0;i<this->cnt;i++)
	{	
		if(this->reg[i].status == 1)
		{
			oncnt++;
		}
	}
	list = "Online Users: ";
	list.append(to_string(oncnt));
	list.append("\n");
	for(int i = 0;i<this->cnt;i++)
	{	
		if(this->reg[i].status == 1)
		{
			list.append(this->reg[i].name);
			list.append("#");
			list.append(this->reg[i].IP);
			list.append("#");
			list.append(to_string(this->reg[i].portnum));
			list.append("\n");
			
		}
	}
	return list;
}

//Struct Related
typedef struct T_para T_para;
struct T_para{
	char *ipaddr;
	int sockfd;
	List *REG;
	SSL *ssl;
	
};

//==================================================

void task(void *arg)
{
	T_para *ptr;
	ptr = (T_para *) arg;
	
	char *c_ipaddr;
	c_ipaddr = ptr->ipaddr;
	int forClientSockfd = ptr->sockfd;
	List *REG;
	REG = ptr->REG;
	
	SSL* ssl;
	ssl = ptr->ssl;
	
	//printf("cipaddr = %s\n",c_ipaddr);
	//printf("clientsock = %d\n",forClientSockfd);
	
	char inputBuffer[16*LEN] = {};
	char modestr[1] = {};
	char message[LEN] = {"Server Received\n"};
	/*string *message;
	string messagetext = "SERVER\n";
	message = &messagetext;*/
	char bad[LEN] = {"Unknown Command:(\n"};
	char bye[LEN] = {"Bye~~See you again :)\n"};
	
	int lognum = -1;
	int usernumber = -1;
	
	
	//printf("FORSOCK: %d\n",forClientSockfd);
	
	string userinfo,onlinelist,list;
	
	while(1)
	{
		printf("====================\n");
		//recv(forClientSockfd,modestr,1,0);
		SSL_read(ssl,modestr,1);
		
		if (strncmp(modestr,"1",1) == 0)
		{
			//printf("Mode 1 : Register\n");
			//recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
			SSL_read(ssl,inputBuffer,sizeof(inputBuffer));
			
			printf("Received from Client: %s\n",inputBuffer);
			//=====if find name return fail=====
			//create user(name,money)
			//add in list
			//cnt++
			
			string c_name_money,c_name,c_money_str,input;
			input = inputBuffer;
			//cout << "inputis =" <<input<<"=\n";
			int pos = 0,pos2 = 0;
			pos = input.find('#');
			c_name_money = input.substr(pos+1);
			pos2 = c_name_money.find('#');
			c_name = c_name_money.substr(0,pos2);
			c_money_str = c_name_money.substr(pos2+1);
			
			//cout << "name and money\n";
			//cout << c_name << "\n" << c_money_str << "\n";
			
			int trynumber = -2;
			trynumber = REG->finduser(c_name);
			if (trynumber == -1)
			{	
				char message1[LEN] = {"100 OK\n"};
				//send(forClientSockfd,message1,sizeof(message1),0);
				SSL_write(ssl,message1,sizeof(message1));
				User u(c_name,atoi(c_money_str.c_str()));
				//u.userinfo();
				REG->reg.push_back(u);
				REG->cnt++;
				//REG->listinfo();
			}
			else
			{
				char message1[LEN] = {"210 FAIL\n"};
				//send(forClientSockfd,message1,sizeof(message1),0);
				SSL_write(ssl,message1,sizeof(message1));
			}
			
			
			
			
		}
		else if (strncmp(modestr,"2",1) == 0)
		{
			//printf("Mode 2 : Login\n");
			//recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
			SSL_read(ssl,inputBuffer,sizeof(inputBuffer));
			
			printf("Received from Client: %s\n",inputBuffer);
			//record name!!!
			//find login user
			//=====if cannot find return fail=====
			//set ip, port, status
			//update online list and send back
			
			string logname,logport_str,ipaddr,input;
			input = inputBuffer;
			int pos = 0;
			int logport = -1;
			pos = input.find('#');
			logname = input.substr(0,pos);
			logport_str = input.substr(pos+1);
			logport = atoi(logport_str.c_str());
			ipaddr = c_ipaddr;
			
			//cout << "name and port\n";
			//cout << logname << "\n" << logport << "\n";
			
			usernumber = REG->finduser(logname);
			//cout << "usernumber\n";
			//cout << usernumber << "\n";
			
			if (usernumber == -1)
			{	
				char message2[LEN] = {"220 AUTH_FAIL\n"};
				//send(forClientSockfd,message2,sizeof(message2),0);
				SSL_write(ssl,message2,sizeof(message2));
			}
			else
			{
				if(REG->reg[usernumber].status == 1)
				{
					char message2[LEN*2] = {"This account has been logged in!\n"};
					//send(forClientSockfd,message2,sizeof(message2),0);
					SSL_write(ssl,message2,sizeof(message2));
				}
				else
				{
								
				REG->reg[usernumber].setonline(ipaddr,logport);
				//cout << "Done\n";
				//REG->listinfo();
				onlinelist = REG->getonlinelist();
				userinfo = REG->reg[usernumber].getuserinfo();
				//cout << userinfo << "\n" << onlinelist;
				list = userinfo;		
				list.append(onlinelist);
				//cout << list;
				
				char message2[LEN * LEN];
				strcpy(message2, list.c_str());
				//send(forClientSockfd,message2,sizeof(message2),0);
				SSL_write(ssl,message2,sizeof(message2));
				}
			}
			
			
		}
		else if (strncmp(modestr,"3",1) == 0)
		{
			//printf("Mode 3 : List\n");
			//recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
			SSL_read(ssl,inputBuffer,sizeof(inputBuffer));
			
			printf("Received from Client: %s\n",inputBuffer);
			//return list(updated)
			
			
			onlinelist = REG->getonlinelist();
			userinfo = REG->reg[usernumber].getuserinfo();
			//cout << userinfo << "\n" << onlinelist;
			list = userinfo;			
			list.append(onlinelist);
			//cout << list;
			
			char message3[LEN * LEN];
			strcpy(message3, list.c_str());
			//send(forClientSockfd,message3,sizeof(message3),0);
			SSL_write(ssl,message3,sizeof(message3));
		}
		else if (strncmp(modestr,"4",1) == 0)
		{
			//printf("Mode 4 : Exit\n");
			//recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
			SSL_read(ssl,inputBuffer,sizeof(inputBuffer));
			
			
			//send(forClientSockfd,bye,sizeof(bye),0);
			SSL_write(ssl,bye,sizeof(bye));
			printf("Received from Client: %s\n",inputBuffer);
			printf("Client Disconnected!\n");
			printf("====================\n");
			//update list
			
			REG->reg[usernumber].setoffline();
			
			close(forClientSockfd);
			SSL_shutdown(ssl);
			SSL_free(ssl);
			pthread_mutex_lock(&lock);
			clientcnt--;
			pthread_mutex_unlock(&lock);
			break;
		}
		else if (strncmp(modestr,"5",1) == 0)
		{
			//printf("Mode 5 : Update money\n");
			char transfer[LEN*10];
			
			//recv(forClientSockfd,transfer,sizeof(transfer),0);
			SSL_read(ssl,transfer,sizeof(transfer));
			
			printf("Received from Client: %s\n",transfer);
			//use record name find user
			//change money
			
			onlinelist = REG->getonlinelist();
			userinfo = REG->reg[usernumber].getuserinfo();
			//cout << userinfo << "\n" << onlinelist;
			list = userinfo;			
			list.append(onlinelist);
			//cout << list;
			
			char message5[LEN * LEN];
			strcpy(message5, list.c_str());
			//send(forClientSockfd,message5,sizeof(message5),0);
			SSL_write(ssl,message5,sizeof(message5));
			
			//update
			
			string sender,receiver,money,temp,input;
			input = transfer;
			//cout << "inputis =" <<input<<"=\n";
			int pos = 0,pos2 = 0;
			pos = input.find('#');
			sender = input.substr(0,pos);
			temp = input.substr(pos+1);
			pos2 = temp.find('#');
			money = temp.substr(0,pos2);
			receiver = temp.substr(pos2+1);
			//cout << "s,m,r:\n";
			//cout << sender << "\n" << money << "\n" << receiver << "\n";
			
			int sint,rint,mint;
			mint = atoi(money.c_str());
			
			sint = REG->finduser(sender);
			rint = REG->finduser(receiver);
			
			if(sint == rint)
			{
				cout << "Self Transfer!\n";
				char transfail[LEN * 2] = {"Transaction Failed: Cannot transfer to yourself\n"};
				SSL_write(ssl,transfail,sizeof(transfail));
			}
			else if(rint == -1)
			{
				cout << "User not found!\n";
				char transfail[LEN * 2] = {"Transaction Failed: User not exist\n"};
				SSL_write(ssl,transfail,sizeof(transfail));
			}
			else if(REG->reg[sint].money < mint)
			{
				cout << "Not enough money!\n";
				char transfail[LEN * 2] = {"Transaction Failed: Not enough money\n"};
				SSL_write(ssl,transfail,sizeof(transfail));
			}
			else
			{
				REG->reg[sint].transfer(&REG->reg[rint],mint);
				//REG->listinfo();
				char transuccess[LEN * 2] = {"Transaction Successful!\n"};
				SSL_write(ssl,transuccess,sizeof(transuccess));			
			}
		}
		else
		{
			//recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
			SSL_read(ssl,inputBuffer,sizeof(inputBuffer));
			
			//send(forClientSockfd,bad,sizeof(bad),0);
			SSL_write(ssl,bad,sizeof(bad));
			printf("Received from Client(unknown command): %s\n",inputBuffer);
		}
	}
}//thread function




int main(int argc , char *argv[])

{
	SSL_CTX* ctx = InitServerCTX();
	DynamicCertificates(ctx);

	threadpool_t *pool;
	pthread_mutex_init(&lock, NULL);
	pool = threadpool_create(THREAD, QUEUE, 0);
	if (pool != NULL)
	{
		//printf("Pool Started with %d threads and queue size of %d\n",THREAD,QUEUE);
	}
	else
	{
		printf("Pool error!\n");
	}
	
	
	//Input Processing
	char* ipnum = argv[1];
	int portnum = atoi(argv[2]);
	if (strncmp(ipnum,"Local",5) == 0)
	{
		ipnum = (char*)"127.0.0.1";
	}
	printf("Listening to %s:%d\n",ipnum,portnum);
	
	//socket的建立
	char inputBuffer[16*LEN] = {};
	char connect[] = {"Server Connected.\n"};
	char message[] = {"Server Received.\n"};
	char bye[] = {"Bye~~See you again :)\n"};
	int sockfd = 0,forClientSockfd = 0;
	sockfd = socket(AF_INET , SOCK_STREAM , 0);

	if (sockfd == -1)
	{
		printf("Fail to create a socket.");
	}

	//socket的連線
	struct sockaddr_in serverInfo,clientInfo;
	unsigned int addrlen = sizeof(clientInfo);
	bzero(&serverInfo,sizeof(serverInfo));

	serverInfo.sin_family = PF_INET;
	serverInfo.sin_addr.s_addr = inet_addr(ipnum);
	serverInfo.sin_port = htons(portnum);
	int binded = 0;
	binded = bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
	if (binded == -1)
	{
		printf("Binding Error!\n");
	}
	
	listen(sockfd,LEN);
	List REG;

	while(1)
	{
		
		forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
		send(forClientSockfd,connect,sizeof(connect),0);
		printf("Client Connected!\n");
		
		
		struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&clientInfo;
		struct in_addr ipAddr = pV4Addr->sin_addr;
		char client_ip_addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ipAddr, client_ip_addr, INET_ADDRSTRLEN);
		
		
		SSL* ssl = SSL_new(ctx);
		SSL_set_fd(ssl, forClientSockfd);
		/* 建立 SSL 連線 */
		if (SSL_accept(ssl) == -1)
		{
		    ERR_print_errors_fp(stderr);
		    close(forClientSockfd);
		    continue;
		}
		else
		{
			ShowCerts(ssl);
			
			T_para para = {
				.ipaddr = client_ip_addr,
				.sockfd = forClientSockfd,
				.REG = &REG,
				.ssl = ssl
			};
			
			//printf("ipaddr: %s\n",client_ip_addr);
			//printf("forsock: %d\n",forClientSockfd);
			if (threadpool_add(pool, &task, &para, 0) != 0)
		    	{
		    		printf("Server is full.\n");
		    	}
		    	else
		    	{
		    		pthread_mutex_lock(&lock);
				clientcnt++;
				pthread_mutex_unlock(&lock);
		    	}
		}
		
	}
	SSL_CTX_free(ctx);
	close(sockfd);
	return 0;
}
