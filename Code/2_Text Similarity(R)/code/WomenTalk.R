# 自訂Cosine Similarity之函數
cos_sim <- function(x,y)
{
  result <- x %*% y /(sqrt(x %*% x)*(sqrt(y %*% y))) 
  return(result[1])
}


## Dcard analysis

# 爬1000篇最新文章，將1000篇文章之id存入向量"idxs"
library(stringr)
library(httr)
idxs <- vector(mode = "character",length = 1000)

req <- GET('https://www.dcard.tw/',
           path = c("_api","forums","girl","posts?popular=false&limit=100"))
idx <- content(req)

for (i in 1:100){
  idxs[i] <- idx[[i]]$id
}
id_end <- idxs[100]

i <- 101
for(loop in 2:10)
{
  str <- paste0("https://www.dcard.tw/_api/forums/girl/posts?popular=false&limit=100&before=", id_end)
  req <- GET(str)
  idx <- content(req) 
  for(count in 1:100)
  {
    idxs[i] <- idx[[count]]$id
    id_end <- idxs[i]
    i <- i + 1
  }
}



# 抽樣500篇文章做分析，同時記下所爬到Dcard最早文章的時間，方便之後PTT做對照
T_end <- content(GET('https://www.dcard.tw/', path = c("_api",
                                                       "posts",idxs[1000])))$createdAt
T_end
S_dcard <- sample(1:1000,size = 500)
idxs<-idxs[S_dcard]


# 藉由idxs中存取的500篇文章之id，將那些文章的「發文時間」、「文章內容」分別存入相對應的vector
time <- vector(mode = "character",length = 500)
dcard_content <- vector(mode = "character",length = 500)
for(i in 1:500)
{
  req2 <- GET('https://www.dcard.tw/', path = c("_api", "posts",idxs[i]))
  
  idx2 <- content(req2)
  time[i] <- idx2$createdAt
  dcard_content[i] <- idx2$content
  
}
dcard_tb <- tibble::tibble(id =idxs, time = time,content = dcard_content)



# 對爬文結果斷詞
# dcard_tb[i]會出現第i篇文章的斷詞結果,文章被空白鍵斷開
library(jiebaR)
library(quanteda)

seg <- worker(symbol = T, bylines = F, user = "words.txt")

for (i in 1:length(dcard_tb$content)){
  dcard_tb$content[i] <-
    dcard_tb$content[i] %>% segment(seg) %>% paste(collapse = " ")
}

# 將斷出的詞存入變數"dcard_toks"
# dcard_toks[i]會出現第i篇文章被斷出來的所有詞彙
dcard_corp <- corpus(dcard_tb, text_field = "content") 
dcard_toks <- tokens(dcard_corp, what = "fasterword" , remove_punct = TRUE,
                     remove_numbers = T, remove_url = T)



# 將每篇文章的詞彙取聯集，存入變數"dcard_all"
dcard_all <- vector("character",100000)
for (i in 1:length(dcard_content)) {
  dcard_all <- union(dcard_all,dcard_toks[[i]])
}


# 查看斷詞聯集的大小
length(dcard_all)


# 製作出dtm
dcard_dtm <- matrix(nrow = length(dcard_content), ncol = length(dcard_all))

for (i in 1:length(dcard_content)) {
  dcard_num  <- vector("numeric",length(dcard_all))
  temp = dcard_toks[[i]]
  for (j in 1:length(dcard_all))
  {
    for(k in 1:length(temp))
    {
      if(temp[k] == dcard_all[j]){
        dcard_num[j] <- dcard_num[j] + 1
      }
    }
  }
  dcard_num <- dcard_num/sum(dcard_num)
  dcard_dtm[i,] <- dcard_num
}

# 任兩篇文章計算其Cosine Similarity
dcard_n <- length(dcard_content)
dcard_cosine_similarity <- vector("numeric", dcard_n*(dcard_n-1)/2 ) 
dcard_index <- 1

for(i in 1:(dcard_n-1)){
  for(j in (i+1):dcard_n){
    dcard_cosine_similarity[dcard_index] <- cos_sim(dcard_dtm[i,],dcard_dtm[j,])
    dcard_index <- dcard_index + 1
  }
}

# 對所有的Cosine Similarity作圖
title <- paste0("cosine similarity(Dcard,girl)\nmedian:",median(dcard_cosine_similarity))
dcard_outlier_values <- boxplot.stats(dcard_cosine_similarity)$out
boxplot(dcard_cosine_similarity, main= title, 
        horizontal=TRUE)

length(dcard_outlier_values)



## PTT analysis

# 初始化爬PTT
library(PTTmineR)

rookie_miner2 <- PTTmineR$new(task.name = "test2")
rookie_miner2


# PTT 女孩版爬到之前Dcard所爬到最早的時間（確保資料同為相同時段）
mine_ptt(ptt.miner = rookie_miner2,
         board = "WomenTalk",
         min.date = "2019-09-24")


# 將爬文結果變成tibble"ptt_tb"
rookie_miner2 %>% 
  export_ptt(export.type = "tbl",
             obj.name = "result")
ptt_content <- result$post_info_tbl$post_content




# 查看被扒的文章總數，取樣500篇
length(ptt_content)

S_ptt <- sample(1:length(ptt_content),size = 500)
ptt_content <- ptt_content[S_ptt]
ptt_tb <- tibble::tibble(content = ptt_content)


# 對爬文結果斷詞
# ptt_tb[i]會出現第i篇文章的斷詞結果,文章被空白鍵斷開
library(jiebaR)
library(quanteda)

seg <- worker(symbol = T, bylines = F, user = "words.txt")

for (i in 1:length(ptt_tb$content)){
  ptt_tb$content[i] <-
    ptt_tb$content[i] %>% segment(seg) %>% paste(collapse = " ")
}

# 將斷出的詞存入變數"ptt_toks"
# ptt_toks[i]會出現第i篇文章被斷出來的所有詞彙
ptt_corp <- corpus(ptt_tb, text_field = "content") 
ptt_toks <- tokens(ptt_corp, what = "fasterword" , remove_punct = TRUE,
                   remove_numbers = T, remove_url = T)




# 為每篇文章的詞彙取聯集，存入變數"ptt_all"
ptt_all <- vector("character",50000)
for (i in 1:length(ptt_content)) {
  ptt_all <- union(ptt_all,ptt_toks[[i]])
}


# 查看斷詞聯集的大小
length(ptt_all)

# 製作出dtm
ptt_dtm <- matrix(nrow = length(ptt_content), ncol = length(ptt_all))

for (i in 1:length(ptt_content)) {
  ptt_num  <- vector("numeric",length(ptt_all))
  temp = ptt_toks[[i]]
  for (j in 1:length(ptt_all))
  {
    for(k in 1:length(temp))
    {
      if(temp[k] == ptt_all[j]){
        ptt_num[j] <- ptt_num[j] + 1
      }
    }
  }
  ptt_num <- ptt_num/sum(ptt_num)
  ptt_dtm[i,] <- ptt_num
}



# 任兩篇文章計算其Cosine Similarity
ptt_n <- length(ptt_content)
ptt_cosine_similarity <- vector("numeric", ptt_n*(ptt_n-1)/2 ) 
ptt_index <- 1

for(i in 1:(ptt_n-1)){
  for(j in (i+1):ptt_n){
    ptt_cosine_similarity[ptt_index] <- cos_sim(ptt_dtm[i,],ptt_dtm[j,])
    ptt_index <- ptt_index + 1
  }
}


# 對所有的Cosine Similarity作圖
title <- paste0("cosine similarity(PTT,WomenTalk)\nmedian:",median(ptt_cosine_similarity))
ptt_outlier_values <- boxplot.stats(ptt_cosine_similarity)$out
boxplot(ptt_cosine_similarity, main=title, 
        horizontal=TRUE)
median(ptt_cosine_similarity)


# 看outlier的數量
length(ptt_outlier_values)
