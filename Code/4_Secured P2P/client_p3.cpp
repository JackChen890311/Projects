#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <iostream>

#define LEN 100
using namespace std;

SSL_CTX* InitClientCTX()
{
    SSL_CTX *ctx;
    /* SSL 庫初始化 */
    SSL_library_init();
    /* 載入所有 SSL 演算法 */
    OpenSSL_add_all_algorithms();
    /* 載入所有 SSL 錯誤訊息 */
    SSL_load_error_strings();
    /* 以 SSL V2 和 V3 標準相容方式產生一個 SSL_CTX ，即 SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_client_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 單獨表示 V2 或 V3標準 */
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stdout);
        abort();
    }
    return ctx;
}
 
 
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
 
//===========================================================
void substr(char* dest, const char* src, unsigned int start, unsigned int cnt)
{
	strncpy(dest, src + start, cnt);
	dest[cnt] = 0;
}

char* itoa(int i, char b[])
{
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}



int main(int argc , char *argv[])
{


	SSL_CTX* ctx = InitClientCTX();
	SSL_CTX* ctx2 = InitServerCTX();

	DynamicCertificates(ctx);
	DynamicCertificates(ctx2);
	
	//Input Processing
	char* ipnum = argv[1];
	int portnum = atoi(argv[2]);
	int logportnum_c = atoi(argv[3]);
	
	if (strncmp(ipnum,"Local",5) == 0)
	{
		ipnum = (char*)"127.0.0.1";
	}
	if (portnum <= 1024 || portnum >= 65536 || logportnum_c <= 1024 || logportnum_c >= 65536)
	{
		printf("Port number must be between 1024~65536!\n");
		return 0;
	}
	
	
	pid_t pid;
	pid = fork();
	
		
	if(pid < 0)
	{
		printf("Fork failed");
	}
	else if (pid == 0) //Child process
	{
		
		printf("Login Port Number: %d\n",logportnum_c);
		
		//socket的建立
		char message_c[LEN] = {"Successful\n"};
		char connect_c[LEN] = {"Client Connected\n"};
		int sockfd_c = 0,forClientSockfd_c = 0;
		sockfd_c = socket(AF_INET , SOCK_STREAM , 0);

		if (sockfd_c == -1)
		{
			printf("Fail to create a socket\n");
		}

		//socket的連線
		struct sockaddr_in serverInfo_c,clientInfo_c;
		unsigned int addrlen_c = sizeof(clientInfo_c);
		bzero(&serverInfo_c,sizeof(serverInfo_c));

		serverInfo_c.sin_family = PF_INET;
		serverInfo_c.sin_addr.s_addr = inet_addr(ipnum);
		serverInfo_c.sin_port = htons(logportnum_c);
		int binded = 0;
		binded = bind(sockfd_c,(struct sockaddr *)&serverInfo_c,sizeof(serverInfo_c));
		if (binded == -1)
		{
			printf("Binding Error!\n");
		}
		else
		{
			//printf("Login Success!\n");
			listen(sockfd_c,LEN);
			
			while(1)
			{
				
				forClientSockfd_c = accept(sockfd_c,(struct sockaddr*) &clientInfo_c, &addrlen_c);
				SSL* ssl2 = SSL_new(ctx2);
				SSL_set_fd(ssl2, forClientSockfd_c);
				/* 建立 SSL 連線 */
				if (SSL_accept(ssl2) == -1)
				{
					cout << "Listen Bad!\n";
					ERR_print_errors_fp(stderr);
					close(forClientSockfd_c);
					continue;
				}
				else
				{
					cout << "Someone transfer to you!\n";
					char inputBuffer_c[LEN] = {};
					SSL_read(ssl2,inputBuffer_c,sizeof(inputBuffer_c));
					//recv(forClientSockfd_c,inputBuffer_c,sizeof(inputBuffer_c),0);
					if (strncmp(inputBuffer_c,"Exit",4) == 0)
					{
						SSL_shutdown(ssl2);
						SSL_free(ssl2);
						close(forClientSockfd_c);
						break;
					}
					else
					{
						SSL_write(ssl2,message_c,sizeof(message_c));
						//send(forClientSockfd_c,message_c,sizeof(message_c),0);
						printf("->Received from other Client: %s\n",inputBuffer_c);
						printf("====================\n");
					}
					SSL_shutdown(ssl2);
					SSL_free(ssl2);
					close(forClientSockfd_c);
				
				}
			}
		}
		close(sockfd_c);
		
	}
	else //Parent process
	{
		
		printf("(Connecting to Server...)\n");
		
		//socket的建立
		int sockfd = 0;
		sockfd = socket(AF_INET , SOCK_STREAM , 0);

		if (sockfd == -1)
		{
			printf("Fail to create a socket\n");
		}

		//socket的連線
		struct sockaddr_in info;
		bzero(&info,sizeof(info));

		info.sin_family = PF_INET;
		info.sin_addr.s_addr = inet_addr(ipnum);
		info.sin_port = htons(portnum);


		int error = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
		if(error == -1)
		{
			printf("Connection error\n");
		}

		char receiveMessage[LEN] = {};
		recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
		printf("%s",receiveMessage);
		

		printf("You're all set!\n");
		//Send a message to server
		int login = 0;
		int logportint = 0;
		int toportint = 0;
		char logname[LEN] = {};
		char logport[LEN] = {};
		
		/* 基於 ctx 產生一個新的 SSL */
		SSL* ssl = SSL_new(ctx);
		SSL_set_fd(ssl, sockfd);
		/* 建立 SSL 連線 */
		if (SSL_connect(ssl) == -1)
		{
			ERR_print_errors_fp(stderr);
		}
		else
		{
			printf("====================\n");
			printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
			ShowCerts(ssl);
		}
		

		while(1)
		{	
			printf("====================\n");
			int mode = 0;
			char message[LEN] = {};
			char message_out[LEN] = {};
			int pos_out = 0;
			char receiveMessage[LEN*LEN] = {};
			while(mode == 0)
			{
				
				char message[LEN] = {};
				fgets(message,LEN,stdin);

				if(message[strlen(message)-1] == '\n')
				{
					message[strlen(message)-1] = '\0';
				}
				
				//command preprocessing
				//mode
				//1: register#jack#8888
				//2: jack#9000
				//3: list
				//4: exit
				//5: jack#2222#bob
				
				int cnt = 0;
				int flag = 0;
				int pos = 0;
				
				for(int i = 0;i < strlen(message);i++)
				{
					message_out[i] = message[i];
					if (message[i] == '#' && flag == 0)
					{
						cnt++;
						flag = 1;
						pos = i;
					}
					else if (message[i] == '#' && flag == 1)
					{
						cnt++;
					
					}
				}
				message_out[strlen(message_out)] = '\0';
				
				if (cnt == 0)
				{
					if (strncmp(message,"List",4) == 0 && strlen(message) == 4)
					{
						mode = 3;
					}
					else if (strncmp(message,"Exit",4) == 0 && strlen(message) == 4)
					{
						mode = 4;
					}
					else
					{
						printf("Invalid Command!\n");
						continue;
					}
				
				}
				else if (cnt == 1)
				{
					mode = 2;
				}
				else if (cnt > 2)
				{
					printf("Invalid Command!\n");
					continue;
				}
				else
				{
					if (strncmp(message,"REGISTER",7) == 0)
					{
						mode = 1;
					}
					else
					{
						mode = 5;
					}
			
				}
				pos_out = pos;
				
			}
			
			char modestr[LEN] = {};
			itoa(mode,modestr);
			char text[LEN] = {};
			char totext[LEN] = {};

			switch(modestr[0])
			{
				case '1':
					SSL_write(ssl,modestr,1);
					//send(sockfd,modestr,1,0);
					sleep(0.2);
					SSL_write(ssl,message_out,sizeof(message_out));
					//send(sockfd,message_out,sizeof(message_out),0);
					SSL_read(ssl,receiveMessage,sizeof(receiveMessage));
					//recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
					printf("->Received from server:\n%s",receiveMessage);
					break;
				case '2':
					if (login == 1)
					{
						printf("You already log in!\n");
						mode = 0;
					}
					else
					{
						
						substr(logname,message_out,0,pos_out);
						substr(logport,message_out,pos_out+1,strlen(message_out)-pos_out);
						logportint = atoi(logport);

						if (logportint != logportnum_c)
						{
							printf("Please log in with same port number!\n");
							mode = 0;
						}
						else
						{
							printf("Logging...\n");
							SSL_write(ssl,modestr,1);
							//send(sockfd,modestr,1,0);
							sleep(0.2);
							SSL_write(ssl,message_out,sizeof(message_out));
							//send(sockfd,message_out,sizeof(message_out),0);
							SSL_read(ssl,receiveMessage,sizeof(receiveMessage));
							//recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
							printf("->Received from server:\n%s",receiveMessage);
							
							if (strncmp(receiveMessage,"220 AUTH",8) == 0 || strncmp(receiveMessage,"This",4) == 0 || strncmp(receiveMessage,"Port",4) == 0)
							{
								printf("Login Error!\n");
								mode = 0;
							}
							else
							{
								login = 1;
							}
							
						}
					}
					break;
				case '3':
					if (login == 1)
					{
						SSL_write(ssl,modestr,1);
						//send(sockfd,modestr,1,0);
						sleep(0.2);
						SSL_write(ssl,message_out,sizeof(message_out));
						//send(sockfd,message_out,sizeof(message_out),0);
						SSL_read(ssl,receiveMessage,sizeof(receiveMessage));
						//recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
						printf("->Received from server:\n%s",receiveMessage);
					}
					else
					{
						printf("Please log in first!\n");
						mode = 0;
					}
					break;
				case '4':
					if (login == 1)
					{
						printf("(Closing Connection...)\n");
						SSL_write(ssl,modestr,1);
						//send(sockfd,modestr,1,0);
						sleep(0.2);
						SSL_write(ssl,message_out,sizeof(message_out));
						//send(sockfd,message_out,sizeof(message_out),0);
						SSL_read(ssl,receiveMessage,sizeof(receiveMessage));
						//recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
						printf("->Received from server:\n%s",receiveMessage);
						SSL_shutdown(ssl);
						SSL_free(ssl);
						close(sockfd);
						logportint = atoi(logport);
					}
					else
					{
						printf("Please log in first!\n");
						mode = 0;
					}
					break;
				case '5':
					if (login == 1)
					{
						
						char ask[LEN] = {"List"};
						char list[LEN*LEN] = {};
						char name[LEN] = {};
						char toname[LEN] = {};
						char toipport[LEN] = {};
						char toport[LEN] = {};
						char sublist[LEN] = {};
						char status[LEN] = {};
						
						substr(name,message_out,0,pos_out);
						substr(text,message_out,pos_out + 1,strlen(message_out)-pos_out);
						
						if (strncmp(name,logname,strlen(logname)) == 0 && strlen(name) == strlen(logname))
						{
							
							
							//comparison
							int secpos = 0;
				
							for(int i = 0;i < strlen(text);i++)
							{
								if (text[i] == '#')
								{
									secpos = i;
									break;
								}
							}
							substr(totext,text,0,secpos);
							substr(toname,text,secpos+1,strlen(text)-secpos);
		
							SSL_write(ssl,modestr,1);
							//send(sockfd,modestr,1,0);
							
							SSL_write(ssl,message_out,sizeof(message_out));
							//send(sockfd,message_out,sizeof(message_out),0);
							SSL_read(ssl,list,sizeof(list));
							//recv(sockfd,list,sizeof(list),0);
							
							
							SSL_read(ssl,status,sizeof(status));
							//recv(sockfd,list,sizeof(list),0);
							printf("->Received from server:\n%s",status);
							
							int index = -1;
							for(int i = 0;i < strlen(list)-strlen(toname);i++)
							{
								int k = i;
								int cntname = 0;
								for(int j = 0;j < strlen(toname);j++)
								{
									
									if(list[k] == toname[j])
									{
										cntname++;
										k++;
									}
									else
									{
										break;
									}
									
								}
								substr(sublist,list,k,10);
								
								if (cntname == strlen(toname) && strncmp(sublist,"#",1) == 0)
								{
									index = i;
								}
							}
							
							
							if (strncmp(toname,logname,strlen(logname)) == 0 && strlen(toname) == strlen(logname))
							{
								//printf("You cannot transfer to yourself!\n");
								mode = 0;
							}
							else if (index == -1)
							{
								//printf("Name not found!\n");
								mode = 0;
							}
							else if (strncmp(status,"Transaction Failed: Not enough money",36) == 0)
							{
								//printf("You are broke!\n");
								mode = 0;
							
							}
							else
							{
								
								int stop = 0;
								for(int i = index+2;i < strlen(list);i++)
								{
									if (list[i] == '\n')
									{
										stop = i;
										break;
									}
								}
								
								char toipport[LEN] = {};
								char toport[LEN] = {};
								substr(toipport,list,index+strlen(toname)+1,stop-index-strlen(toname)-1);

								
								for(int i = 0;i < strlen(toipport);i++)
								{
									if (toipport[i] == '#')
									{
										secpos = i;
										break;
									}
								}
								substr(toport,toipport,secpos+1,stop-secpos-1);
								toportint = atoi(toport);
								logportint = toportint;
								
							}
							
							
							
							
						}
						else
						{
							printf("Name Error!\n");
							mode = 0;
						}
					}
					else
					{
						printf("Please log in first!\n");
						mode = 0;
					}
					break;
				default:
					printf("The code is wrong :(((\n");
			
			}

			if (mode == 4 || mode == 5)
			{	
			
				//send to child process or others
				
				char res_c[LEN] = {};
				char exit[LEN] = {"Exit\0"};
	
				
				//socket的建立
				int sockfd_tc = 0;
				sockfd_tc = socket(AF_INET , SOCK_STREAM , 0);

				if (sockfd_tc == -1)
				{
					printf("Fail to create a socket(closing)\n");
				}

				//socket的連線
				struct sockaddr_in info_tc;
				bzero(&info_tc,sizeof(info_tc));
				info_tc.sin_family = PF_INET;
				info_tc.sin_addr.s_addr = inet_addr(ipnum);
				info_tc.sin_port = htons(logportint);


				int err_tc = connect(sockfd_tc,(struct sockaddr *)&info_tc,sizeof(info_tc));
				if(err_tc == -1)
				{
					printf("Connection error(closing)\n");
				}
				
				
				/* 基於 ctx 產生一個新的 SSL */
				SSL* ssl3 = SSL_new(ctx);
				SSL_set_fd(ssl3, sockfd_tc);
				/* 建立 SSL 連線 */
				if (SSL_connect(ssl3) == -1)
				{
					ERR_print_errors_fp(stderr);
					printf("Connected with encryption error(closing)\n");
				}

				
				if (mode == 4 && login == 1)
				{
					SSL_write(ssl3,exit,sizeof(exit));
					//send(sockfd_tc,exit,sizeof(exit),0);
					SSL_shutdown(ssl3);
					SSL_free(ssl3);
					close(sockfd_tc);
					printf("====================\n");
					printf("(Waiting...)\n");
					wait(NULL);
					printf("Close all the connection successfully!\n");
					printf("~~Goodbye :)~~\n");
					//free(ptr);
					break;
				}
				else if (mode == 5)
				{
	
					SSL_write(ssl3,totext,sizeof(totext));
					//send(sockfd_tc,totext,sizeof(totext),0);
					SSL_read(ssl3,res_c,sizeof(res_c));
					//recv(sockfd_tc,res_c,sizeof(res_c),0);
					printf("Sending status: %s",res_c);
					SSL_shutdown(ssl3);
					SSL_free(ssl3);
					close(sockfd_tc);
					
			
				}
			}
		}
	}
	
	SSL_CTX_free(ctx);
	SSL_CTX_free(ctx2);
	return 0;
}
