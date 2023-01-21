# 此份 Code 為面試展示用，請勿作為其他用途

import pygame,time
from pygame.locals import * 
from pygame import mixer
import tkinter as tk
import tkinter.font as tkf

# 路徑設定
Path = "C:\\Users\\User\\Documents\\GitHub\\Projects\\Code\\Taiko(Python)\\"

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
mixer.music.load(Path+'sound\\drum.mp3')             # 設定背景音樂
mixer.music.play()

pygame.init()                                   # 初始化 pygame
font  = pygame.font.SysFont(None,60)            # None:使用系統預字型
font2 = pygame.font.SysFont(None,30)            # None:使用系統預字型
screen = pygame.display.set_mode((WINDOWWIDTH, WINDOWHEIGHT+40))    
pygame.display.set_caption("太鼓達人")          # 設定視窗標題
screen.fill(BACKGROUNDCOLOR) 
image = pygame.image.load(Path+'imgs\\logo_taiko.png')     # 載入圖片
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
        f1 = tkf.Font(size = 15,family = "Courier New")
        f2 = tkf.Font(size = 28,family = "Courier New")
        self.txt = tk.Text(self,height = 1,width = 15,font = f2)
        self.txt.grid(row = 0,column = 0,columnspan = 15,sticky = s)
        self.lbl = tk.Label(self,text = "請輸入歌曲(e.g. Shape of You)",height = 1,width = 20,font = f1)
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
    mixer.music.load(Path+'music\\%s.mp3'%songstr) # 載入背景音樂
    mixer.music.play()
    
    background = pygame.Surface(screen.get_size())  # 建立畫布
    background.fill(BACKGROUNDCOLOR)                # 顯示畫布為白色
    image = pygame.image.load(Path+'imgs\\drum_Basemap.png')   # 載入圖片
    background.blit(image,(0,0))                    # 繪製圖片
                                
    """以下為目前歌曲的鼓(已對過節奏)，若有要增減或修改從此處"""
    if songstr == "find you":
      new_drums1 = [2114, 2858, 3146, 3298, 3618, 3866, 3970, 4074, 4178, 4746, 5186, 5658, 5954, 6242, 6490, 6754, 6986, 7394, 7874, 8090, 8186, 8410, 8538, 8890, 9050, 9234, 9650, 10114, 10426, 10586, 10994, 11242, 11410, 11898, 12114, 12282, 12946, 13282, 13738, 13794, 13850, 13906, 13962] 
      new_drums2 = [2906, 3098, 3354, 3578, 4290, 4402, 4506, 4618, 4962, 5418, 5698, 5914, 7650, 7754, 7978, 8314, 8770, 8938, 9098, 9290, 10162, 10378, 10626, 11042, 11202, 11466, 12074, 12250, 12434, 12786, 13122]
    
    elif songstr == "red":
      new_drums1 = [706, 802, 1186, 1586, 1706, 2530, 2634, 3026, 3426, 3546, 4338, 4450, 4578, 4682, 4810, 4930, 5050, 5146, 6178, 6226, 6290, 7082, 7202, 7314, 7434, 8010, 8170, 8298, 8394, 8458, 8682, 8730, 8914, 9042, 9154, 9266, 9386, 9490, 9602, 9714, 9842, 9946, 10330, 10746, 10858, 11658, 11826, 11946, 12042, 12106, 12338, 12402, 12586, 12746, 12850, 12962, 13026, 13146, 13266, 13378, 13490, 13610, 13722, 13850, 13978, 14090, 14194, 14298, 14410, 14530, 14650]
      new_drums2 = [754, 1138, 1242, 1642, 2586, 2978, 3082, 3474, 5258, 5370, 5482, 5602, 5706, 5818, 5938, 6050, 6634, 6690, 6738, 7546, 7650, 7762, 7874, 8114, 8346, 8570, 8802, 9890, 10282, 10386, 10794, 10978, 11098, 11210, 11778, 11994, 12226, 12466, 12698, 12906]
      
    elif songstr == "shape of you":
      new_drums1 = [1954, 2146, 2210, 2442, 2506, 2738, 2922, 3106, 3330, 3394, 3634, 3698, 3914, 4106, 4290, 4410, 4530, 4594, 4706, 4810, 5482, 5602, 5714, 5786, 5906, 6010, 6202, 6362, 6594, 6682, 6762, 6834, 6898, 6970, 7050, 7122, 7194, 7850, 7994, 8162, 8282, 9058, 9218, 9426, 9578, 9650, 9874, 10066, 10242, 10370, 10482, 10554, 10658, 10778, 11450, 11522, 11602, 11674, 11746, 11818, 11898, 11970, 12634, 12786, 12930, 13082, 13234, 13306, 13378, 13450, 13530]
      new_drums2 =  [2034, 2330, 2626, 2802, 3034, 3218, 3514, 3810, 3986, 4226, 4882, 5002, 5106, 5170, 5298, 5402, 6082, 6298, 6490, 7274, 7346, 7418, 7490, 7570, 7642, 7714, 7778, 8466, 8546, 8618, 8690, 8762, 8834, 8906, 8970, 9130, 9282, 9354, 9514, 9770, 9946, 10170, 10858, 10994, 11130, 11282, 12042, 12114, 12186, 12258, 12338, 12410, 12482, 12546, 12714, 12858, 12994, 13146]
    
    elif songstr == "beautiful madness":
      new_drums1 = [1762, 1850, 1970, 2034, 2194, 2362, 2434, 2474, 2570, 2610, 2650, 2730, 2810, 3530, 3610, 3690, 3770, 3842, 3922, 3994, 4058, 4138, 4258, 4338, 4466, 4538, 4618, 4698, 4770, 4834, 4914, 4986, 5346, 5578, 5650, 5730, 5882, 6122, 6410, 6482, 6562, 6634, 6714, 6786, 6938, 7082, 7226, 7378, 7722, 8650, 8786, 8850, 9202, 9514, 9586, 9810, 9898, 10042, 10274, 10570, 10642, 10874, 11026, 11314, 11386, 11778, 12074, 12218, 12514, 12674, 12810, 12962, 13138, 13714]
      new_drums2 = [1826, 1890, 2002, 2114, 2146, 2274, 2306, 2514, 2914, 2994, 3066, 3154, 3226, 3306, 3370, 3442, 4226, 4378, 5050, 5130, 5194, 5258, 5418, 5506, 5810, 5962, 6042, 6186, 6258, 6338, 8482, 9074, 9370, 9674, 9962, 10130, 10418, 10722, 11170, 11466, 11618, 11930, 12002, 12370, 13274, 13426]
      
    elif songstr == "neurose":
      new_drums1 = [986, 1058, 1546, 1666, 2170, 2386, 2802, 2986, 3346, 3482, 3594, 4570, 4690, 4802, 5202, 5786, 5906, 6394, 6546, 7002, 7122, 7642, 7730, 8834, 8946, 9130, 9290, 10042, 10154, 10642, 10682, 10722, 10802, 10914, 11850, 12178, 12338, 12778, 13218, 13346, 13714, 13914, 14282, 14410, 14522]
      new_drums2 = [1026,1178, 1586, 1778, 2146, 2282, 2762, 2882, 3962, 4090, 4202, 5178, 5306, 5410, 6034, 6226, 6698, 6850, 7226, 7602, 7842, 8218, 8250, 8346, 8474, 9426, 9562, 9730, 9898, 10266, 11290, 11394, 11562, 11722, 11986, 12250, 12402, 12474, 12594, 13090, 13250, 13682, 13802, 14314] 
      
    elif songstr == "calling you" :
      new_drums1 =  [1666, 1754, 1826, 1906, 1986, 2058, 2130, 2202, 2866, 2914, 2978, 3058, 3130, 3202, 3266, 3346, 3418, 3482, 3562, 3626, 3706, 3778, 3858, 3938, 4098, 4250, 4330, 4482, 4626, 4698, 4786, 4858, 4930, 5010, 5074, 5146, 5298, 5450, 5834, 5978, 6098, 6194, 6242, 6506, 6690, 6810, 6898, 7034, 7186, 7610, 7682, 7714, 7778, 7818, 8218, 8298, 8338, 8402, 8434, 8594, 8698, 8770, 8858, 8930, 9018, 9082, 9154, 9250, 9554, 9650, 9722, 10018, 10106, 10138, 10210, 10242, 10330, 10402, 10434, 10506, 10538, 10634, 10802, 10954, 11098, 11242, 11858, 11930, 12018, 12090, 12170, 12306, 13210, 13642]
      new_drums2 = [2274, 2346, 2418, 2498, 2570, 2642, 2722, 2786, 4018, 4178, 4402, 4554, 5226, 5378, 5482, 5562, 5634, 5906, 6058, 6426, 6578, 6618, 6770, 7922, 7994, 8034, 8098, 8130, 8530, 8634, 9362, 9434, 11402, 11554, 11690, 12618, 12690, 12762, 12834, 12914, 13426, 13874]      
    
    image = pygame.image.load(Path+"imgs\\drum1.png")   # 載入背景圖片
    drumr_image = pygame.transform.scale(image, (50,50))             # 重新設定鼓的大小(紅)

    image = pygame.image.load(Path+"imgs\\drum2.png")   # 載入背景圖片
    drumb_image = pygame.transform.scale(image, (50,50))             # 重新設定鼓的大小(藍)

    good = pygame.Rect(335,190,100,100)                                  # 存放 Rect 物件，記錄good的位置及大小    
    image = pygame.image.load(Path+"imgs\\good.png")    # 載入背景圖片
    good_image = pygame.transform.scale(image, (100,100))               # 重新設定good的大小  
    
    bad = pygame.Rect(360,190,100,100)                                  # 存放 Rect 物件，記錄bad的位置及大小    
    image = pygame.image.load(Path+"imgs\\bad.png")     # 載入背景圖片
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
                  # new_drums1.append(370 + 8 * timing)
                if event.key == 307:
                  pressb = 1
                  # new_drums2.append(370 + 8 * timing)
        screen.blit(background,(0,0))               # 重繪視窗
        
        max = 99999999
        for i in range(len(new_drums1)):#紅鼓的運行及打擊            
            new_drums1[i] -= dx                                    
            if pressr == 1:
              if (abs(min(new_drums1) - 375)) <= 30:#若按下且在範圍內
                new_drums1[new_drums1.index(min(new_drums1))] = max
                score += 5 * (1 + combo / 100)#加上combo的計分，越多越高分  
                screen.blit(good_image, good) 
                pressr = 0
                combo += 1
              elif (abs(min(new_drums1) - 375)) >= 30:#若按下但不在範圍內
                screen.blit(bad_image, bad) 
                pressr = 0
                combo = 0
            if min(new_drums1) <= 340:#若超過
              new_drums1[new_drums1.index(min(new_drums1))] = max
              screen.blit(bad_image, bad)
              combo = 0
            screen.blit(drumr_image, pygame.Rect(new_drums1[i],220,50,50))
       
        for r in range(len(new_drums2)):#藍鼓的運行及打擊
            new_drums2[r] -= dx 
            if pressb == 1:
              if (abs(min(new_drums2) - 375)) <= 30:
                new_drums2[new_drums2.index(min(new_drums2))] = max
                score += 3 * (1 + combo / 100)  
                screen.blit(good_image, good) 
                pressb = 0
                combo += 1
              elif (abs(min(new_drums2) - 375)) >= 30:
                screen.blit(bad_image, bad) 
                pressb = 0
                combo = 0
            if min(new_drums2) <= 340:
              new_drums2[new_drums2.index(min(new_drums2))] = max
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
    mixer.music.load(Path+'gameover.mp3')
    mixer.music.play()
    # print(new_drums1,new_drums2)

    screen = pygame.display.set_mode((WINDOWWIDTH, WINDOWHEIGHT+40)) 
    screen.fill(BACKGROUNDCOLOR) 
    image = pygame.image.load(Path+"score_Basemap.png")  # 載入圖片
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



