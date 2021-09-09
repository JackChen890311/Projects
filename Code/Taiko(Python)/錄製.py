# Change Directory Before Running
# 此份 Code 為面試展示用，請勿作為其他用途

import pygame,random,sys,time
from pygame.locals import * 
from pygame import mixer
import tkinter as tk
import tkinter.font as tkf

WINDOWWIDTH  = 1200                             # 設定視窗大小
WINDOWHEIGHT = 600
BACKGROUNDCOLOR = (255, 255, 255)               # 設定視窗背景顔色
Running = True

def waitForPressKey():                          # 等待使用者按鍵
    while True:
        for event in pygame.event.get():
            if event.type == QUIT:              # 直接關閉程式視窗
                pygame.quit()
            if event.type == KEYDOWN:               
                return

def waitForYN():                                # 等待使用者按y/n鍵
    global Running    
    while True:
        for event in pygame.event.get(): 
            if event.type == pygame.QUIT:       # 直接關閉程式視窗
                pygame.quit()    
        keys = pygame.key.get_pressed()         # 檢查按鍵被按
        if keys[pygame.K_y]:                    # 按下 y             
            Running = True
            return
        if keys[pygame.K_n]:                    # 按下 n             
            Running = False
            return
                
def drawText(text, font, surface, x, y):        # 繪製文字到視窗中
    textobj = font.render(text, 1, (128, 0, 128))
    textrect = textobj.get_rect()
    textrect.topleft = (x,y)
    surface.blit(textobj,textrect)

mixer.init()                                    # 建立 Soumd 物件
mixer.music.load('C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\drum.mp3')             # 設定背景音樂
mixer.music.play()

pygame.init()                                   # 初始化 pygame
font  = pygame.font.SysFont(None,60)            # None:使用系統預字型
font2 = pygame.font.SysFont(None,30)            # None:使用系統預字型
screen = pygame.display.set_mode((WINDOWWIDTH, WINDOWHEIGHT+40))    
pygame.display.set_caption("太鼓達人")          # 設定視窗標題
screen.fill(BACKGROUNDCOLOR) 
image = pygame.image.load('C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\logo_taiko.png')     # 載入圖片
screen.blit(image,(0,0))                        # 繪製圖片
drawText('Press a key to start.', font, screen, 400,600)
pygame.display.update()

waitForPressKey()                               # 等待使用者按鍵

while Running: 
    #用tkinter製作的歌曲選單， 輸入後按旁邊的按鈕，關閉視窗即可
    class asksong(tk.Frame):                    
      def __init__(self):
        tk.Frame.__init__(self)
        self.grid()
        self.create()
      def create(self):
        s = tk.NE + tk.SW
        f1 = tkf.Font(size = 36,family = "Courier New")
        f2 = tkf.Font(size = 28,family = "Courier New")
        self.txt = tk.Text(self,height = 1,width = 15,font = f2)
        self.txt.grid(row = 0,column = 0,columnspan = 15,sticky = s)
        self.lbl = tk.Label(self,text = "請輸入歌曲",height = 1,width = 20,font = f1)
        self.lbl.grid(row = 1,column = 0,columnspan = 20,sticky = s)
        self.btn = tk.Button(self,text = "確認",height = 1,width = 5,font = f2,command = self.clickbtn)
        self.btn.grid(row = 0,column = 16,columnspan = 5,sticky = s)
      def clickbtn(self):
        global song
        song = self.txt.get("1.0",tk.END)      #將輸入值回傳
    window = asksong()
    window.master.title("Song")
    window.mainloop()

    song = str(song)
    song = song.lower()
    l = len(song)
    songstr = song[0:l-1]                      #字串處理(因textbox多一個\n，並變成小寫)
    mixer.music.load('C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\music\\%s.mp3'%songstr) # 載入背景音樂
    mixer.music.play()
    
    background = pygame.Surface(screen.get_size())  # 建立畫布
    background.fill(BACKGROUNDCOLOR)                # 顯示畫布為白色
    image = pygame.image.load('C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\drum_Basemap.png')   # 載入圖片
    background.blit(image,(0,0))                    # 繪製圖片
                                
    new_drums1 = []
    new_drums2 = []
 
    image = pygame.image.load("C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\drum1.png")   # 載入背景圖片
    drumr_image = pygame.transform.scale(image, (50,50))             # 重新設定鼓的大小(紅)

    image = pygame.image.load("C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\drum2.png")   # 載入背景圖片
    drumb_image = pygame.transform.scale(image, (50,50))             # 重新設定鼓的大小(藍)

    good = pygame.Rect(335,190,100,100)                                  # 存放 Rect 物件，記錄good的位置及大小    
    image = pygame.image.load("C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\good.png")    # 載入背景圖片
    good_image = pygame.transform.scale(image, (100,100))               # 重新設定good的大小  
    
    bad = pygame.Rect(360,190,100,100)                                  # 存放 Rect 物件，記錄bad的位置及大小    
    image = pygame.image.load("C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\bad.png")     # 載入背景圖片
    bad_image = pygame.transform.scale(image, (50,30))                  # 重新設定bad的大小    
    # print(drums)
      
    dx = 8                                          # 鼓移動的間隔
    score = 0                                       # 得分  
    clock = pygame.time.Clock()                     # 建立時間元件物件
    startTime=time.time()
    endTime = time.time()
    pressr = 0   #按下的參數(紅)
    pressb = 0   #按下的參數(藍)
    timing = 0
    combo = 0    #當前combo
    maxcombo = 0 #最大combo

 
    while (endTime-startTime <= 60):                # 設定遊戲時間
        clock.tick(30)                              # 設定while迴圈每秒執行30次
        timing += 1 
        for event in pygame.event.get():
            if event.type == QUIT:                  # 關閉程式視窗
                pygame.quit()
            elif event.type == KEYUP:
                if event.key == 308:
                  pressr = 1
                  new_drums1.append(370 + 8 * timing)
                if event.key == 307:
                  pressb = 1
                  new_drums2.append(370 + 8 * timing)
        screen.blit(background,(0,0))               # 重繪視窗
        
        max = 99999999
        for i in range(len(new_drums1)):#紅鼓的運行及打擊            
            # new_drums1[i] -= dx                                    
            if pressr == 1:
              if (abs(min(new_drums1) - 375)) <= 25:#若按下且在範圍內
                # new_drums1[new_drums1.index(min(new_drums1))] = max
                score += 5 * (1 + combo / 100)#加上combo的計分，越多越高分  
                screen.blit(good_image, good) 
                pressr = 0
                combo += 1
              elif (abs(min(new_drums1) - 375)) >= 25:#若按下但不在範圍內
                screen.blit(bad_image, bad) 
                pressr = 0
                combo = 0
            if min(new_drums1) <= 340:#若超過
              # new_drums1[new_drums1.index(min(new_drums1))] = max
              screen.blit(bad_image, bad)
              combo = 0
            screen.blit(drumr_image, pygame.Rect(new_drums1[i],220,50,50))
       
        for r in range(len(new_drums2)):#藍鼓的運行及打擊
            # new_drums2[r] -= dx 
            if pressb == 1:
              if (abs(min(new_drums2) - 375)) <= 25:
                # new_drums2[new_drums2.index(min(new_drums2))] = max
                score += 3 * (1 + combo / 100)  
                screen.blit(good_image, good) 
                pressb = 0
                combo += 1
              elif (abs(min(new_drums2) - 375)) >= 25:
                screen.blit(bad_image, bad) 
                pressb = 0
                combo = 0
            if min(new_drums2) <= 340:
              # new_drums2[new_drums2.index(min(new_drums2))] = max
              screen.blit(bad_image, bad)
              combo = 0
            screen.blit(drumb_image, pygame.Rect(new_drums2[r],220,50,50))
            
        if maxcombo < combo:#刷新maxcombo
          maxcombo = combo   
          
        msgstr1 = str(int(score))                       # 分數
        msg1 = font.render(msgstr1,True,(255,0,0))      # 第1個參數是顯示的文字；第2個參數:True比較平滑；第3個參數是字體的颜色。
        screen.blit(msg1,(210,230))                     # 顯示分數
        msgstr2 = str(combo)                            # combo
        msg2 = font.render(msgstr2,True,(0,0,255))      
        screen.blit(msg2,(210,190))                     # 顯示combo
        pygame.display.update()                         # 更新視窗
        endTime=time.time()
   
    # 遊戲結束，停止播放背景音樂，顯示分數
    pygame.mixer.music.stop()
    mixer.music.load('C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\gameover.mp3')
    mixer.music.play()
    print("!!!!!It's the data of",songstr,"!!!!!")
    print("Red:",new_drums1)
    print("Blue:",new_drums2)

    screen = pygame.display.set_mode((WINDOWWIDTH, WINDOWHEIGHT+40)) 
    screen.fill(BACKGROUNDCOLOR) 
    image = pygame.image.load("C:\\Users\\USER\\Desktop\\Jack\\107-1course\\PBC\\Project\\final\\score_Basemap.png")  # 載入圖片
    background.blit(image,(0,0))                    # 繪製圖片
    screen.blit(background,(0,0))                   # 重繪視窗
    
    if score >= 300 and score < 500:
        level = "Good!"            # 分數 300~499 Good!
    elif score >= 500:
        level = "Excellent!"       # 分數 >500 Excellent!
    else:
        level = "OK!"              # 分數 <300 OK!

    msgstr1 = "YOUR SCORE : " + str(int(score))  
    msg1 = font.render(msgstr1,True,(255,0,0))      
    screen.blit(msg1,(550,170))                     
    msg2 = font.render(level,True,(0,128,128))
    screen.blit(msg2,(550,250)) 
    msgstr3 = "YOUR MAX COMBO : " + str(maxcombo)
    msg3 = font.render(msgstr3,True,(0,0,255))
    screen.blit(msg3,(550,210)) 
    """以上為顯示分數、combo、評價"""
    
    pygame.display.update()    # 更新視窗
  
    drawText('PLAY AGAIN (y/n)?', font, screen, 430, 600)    
    pygame.display.update()
    # 按下y鍵再玩一次，n鍵離開。
    waitForYN()
        
mixer.music.stop()
pygame.quit()



