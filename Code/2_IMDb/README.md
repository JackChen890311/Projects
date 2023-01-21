# IMDb Movie Ratings Prediction and Actor Embedding Learning by Word2Vec
*Python: sklearn/gensim/torch/pandas*

## 摘要
使用 Python 中 Gensim 套件提供的 Word2Vec 來進行 Actor Embedding 的學習。  
使用 Python 中 Sklearn 套件裡數種機器學習模型進行評分預測。  
使用 Python 中 Pytorch 套件建立神經網路進行評分預測。  
建議閱讀 Slides 即可。  
（本人負責部分為 Actor Embedding Learning 部分：3_Actor2Vec.ipynb） 

## 原始碼說明
1_Prep_Data：資料準備及清洗  
2_EDA：資料探索分析  
3_Actor2Vec：使用 Word2Vec 將演員資訊轉為向量  
4_Models_Regression：使用演員資訊與其他資訊預測電影評分並分析結果（數種模型）  
5_Models_RF：使用演員資訊與其他資訊預測電影評分並分析結果（Random Forest）  
6_Models_NN：使用演員資訊與其他資訊預測電影評分並分析結果（Neural Network）  

## 方法說明
透過 Word2Vec 的原理，將每個演員視為 Word，每部電影視為 Article。  
學出每個演員的 Embedding，並透過結果來分析演員的性質（e.g.動作片、搞笑諧星...）。  
透過上述演員資訊與其他電影資訊，使用多種不同預測模型。  
比較模型之間的準確度，以及輸入資訊不同對預測結果的影響。  
