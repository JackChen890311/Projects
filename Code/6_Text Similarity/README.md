# Text Similarity Analysis on PTT & Dcard
*R: stringr/httr/jiebar/quanteda/pttminer*


## 摘要
使用 NLP 技術分析 PTT 與 Dcard 上不同主題論壇的文章相似度。  
建議閱讀 Slides 即可。

## 原始碼與方法說明

* 一共有五份文件：
	* Gossiping.R
	* NTU.R
	* WomenTalk.R
	* MakeUp.R
	* words.txt

其中前四個R文件內容大致相同，皆為由Dcard、PTT爬下數篇文章後，  
進行斷詞、製作詞頻表並以Cosine Similarity進行分析。  
而斷詞的過程有用到我們自行建立的語料庫，即為第五個文件:words.txt。  
  
其中詳細的進行方式皆附在各檔案註解中，進行方式幾乎完全相同，  
僅有因各版發文頻率不同而造成同時段內發文差異過大才會進行微調。  
（微調時遵守兩大原則：時間區段相同、抽樣數相同）

* PTT NTU板 與 Dcard 台灣大學板：  
	 先由Dcard爬下最新1000篇文，查看其最早日期，再由PTT爬至該日期以確保所分析之時間區段相同。由兩處所爬得之文章皆多於欲分析之文章數500篇，故皆取樣500篇進行分析。
     
* PTT WomenTalk板 與 Dcard女孩板：  
	先由Dcard爬下最新1000篇文，查看其最早日期，再由PTT爬至該日期以確保所分析之時間區段相同。由兩處所爬得之文章皆多於欲分析之文章數500篇，故皆取樣500篇進行分析。

* PTT Gossiping板 與 Dcard時事板：  
	因 PTT Gossiping板發文量過大(一天可能有二、三千篇文章)，與Dcard 時事板發文量（一天約一百篇）有顯著差異，以原本方式將難以進行。因此改為於兩處各爬取兩天份量的文章進行分析。兩天內Dcard 時事板僅有186篇發文量，故各取樣186篇進行分析。  
    
* PTT MakeUp板 與 Dcard美妝板：
	因 PTT Gossiping板發文量過少，與Dcard 時事板發文量有顯著差異，以原本方式將難以進行。（在Dcard美妝板發文量一千篇的時間區段中，PTT MakeUp板僅有211篇發文。）因此改為先由Dcard爬下最新1000篇文，查看其最早日期，再由PTT爬至該日期發現僅有211篇發文，故亦將由Dcard美妝板爬到之1000篇貼文取樣211篇進行分析。
    
    