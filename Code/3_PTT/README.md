# PTT Tag - Adding Meaningful Tags for PTT Post
*Python: request/beautifulsoup/sklearn/monpa/pandas/tkinter*

## 摘要
使用 NLP 技術分析 PTT 股版文章探討議題，並為每篇加上 Hashtag。  
建議閱讀 Slides 或 Report 皆可。  
此專案未附上原始碼請見諒。

## 方法說明
1. 爬取 PTT 股版指定時間範圍內文章
2. 將文章做斷詞前處理，並計算 tf-idf
3. 使用 K-means 將所有文章進行分群
4. 使用 Chi-Square Score 針對每群進行 Feature Selection
5. 從每群挑出的 Feature 即可大致看出每群討論主題
6. 再使用上述 Feature 為文章進行標註（Hashtag）