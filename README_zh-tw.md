Nico Comment Plugin
===========
Open Broadcaster Software(OBS)插件。把聊天室的訊息以Niconico的彈幕型式顯示在OBS的畫面中。

系統需求
===========
Open Broadcaster Software. (編譯中用到OBS Ver 0.5.5.4.裡面附的OBSApi)
Visual C++ 2010 可轉散發套件 

更新紀錄
===========
 - 2014.06.21 Version 0.1.2.3 alpha
   - 完全消除啟動的延遲時間。在Tick()中才觸發連線的判定。
   - 暱稱現在有自己的字型設定和顏色。
   - 暱稱使用Justin/Twitch色彩時，可以自動選擇適合的外框線顏色。
   - 加上日文版介面翻譯。
 
 - 2014.06.18 Version 0.1.2.2 alpha
   - 穩定性大幅提升。
   - 完全移除掉計時器的部分。用OBS內建的Tick()函式來觸發需要計時器的事件。
   - 重寫執行緒的部分，確保執行緒的執行狀態。.
   - 從Socket接收資料的時候加上20秒的上限時間。
 
 - 2014.06.13 Version 0.1.2 alpha
   - 穩定性提升 - 不過在不正常斷線的時候還是有些問題。
   - 在關閉IRCBot的時候，加上20秒強迫關閉執行緒的判斷。
   - 加入匿名登入選項。
   - 加入顯示暱稱的選項，並且可以套用Justin/Twitch的暱稱顏色。
   - 用一個新的亂數產生方法去選擇合適的行來放置新訊息。
   - 重新製作了設定的對話框，加上英文和中文的訊息翻譯。
   - Twitch和Justin.tv現在都可以用 port 6667/443/80。
   - 修正第一次打開設定對話框時沒辦法設定顏色的問題。
   - 用Linked List取代原本的Vector，大幅增進效能。
   - 移除掉IRCBot中的SendThread和ReceivedThread，把所需要的功能直接寫進IRCMsgThread。
   - 重寫StopThread 的函式。用WaitForSingleObject來確保Thread結束。

 - 2014.06.13 Version 0.1.1 alpha
   - 加上緩衝材質的時間，非常大幅的提升效能。
   - 材質更新的頻率從每個畫格更新一次變成每秒更新一次。
   - 從逐句渲染改成逐字渲染，現在可以處理很長的句子。
   - 一秒內不會出現在畫面上的字就不會被渲染出來。
   - 用OBS內建的Log()來取代其他的所有檔案紀錄。
   - 把頻道的字串自動轉換成小寫。
   - 忽略 Nightbot and Moobot 的訊息
   - 重新計算了訊息捲動的速度；10表示字會出現在畫面上五秒
   - 把JTV的連線用Port鎖在6667
   
 - 2014.06.11 Version 0.1 alpha
   - 第一個釋出版本。有很多需要修正的地方
   
安裝
========
把NicoCommentPlugin.dll和NicoCommentPlugin資料夾一起放進去plugins裡面。然後打開OBS，在來源的地方按右鍵新增就會看到"Nico彈幕插件"

授權
========
GNU GPL v2

下載
https://mega.co.nz/#F!MBESTYoJ!vyWh9fJ45Rbdkwf-S_fIIw

原始碼
==========
Nico Comment Plugin(GitHub):
https://github.com/Appendko/NicoCommentPlugin

作者
==========
鴉片(Append Huang)
Append@gmail.com

參考資料
==========
文字渲染的函示是參考OBS內建的文字來源。
https://github.com/jp9000/OBS/

IRCBot的架構主要參考雖小臉世界(Fuunkao Sekai)撰寫的JTChat。
https://github.com/fuunkaosekai/JTChat



