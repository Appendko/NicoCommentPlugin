Nico Comment Plugin
===========
A plugin of Open Broadcaster Software(OBS) to display the chat as Niconico-style comments.

System requirement
===========
Open Broadcaster Software. (I compiled with OBSApi in OBS Ver 0.5.5.4.)
Visual C++ 2010 Runtime Redistributable package

ChangeLog
===========
 - 2014.06.18 Version 0.1.2.2 alpha
   - Improved stability
   - Removed all Timer functions. use Tick() in OBS to trigger the events instead.
   - Rewrite the Thread part to handle the status of the thread correctly.
   - Add the timeout time when receiving data from socket.
 
 - 2014.06.13 Version 0.1.2 alpha
   - Improved stability - although still buggy on accident disconnection
   - Add a 20sec timeout to force terminate unclosed thread while closing IRCBot.
   - Added an option of anonymous login.
   - Added the feature to display message with nickname, using the color from Twitch/Justin.tv.
   - Use a new randomize function to choose the row to draw the message
   - Redesigned a new configurations dialog, with EN and TW localizations.
   - Both Twitch and Justin.tv can use port 6667/443/80 now.
   - Fixed the bug of unable to choose the color while opening the dialog the first time .
   - Replaced the Vector container to Linked List to enhance the performance
   - Removed the SendThread and ReceivedThread in IRCBot; rewrited the IRCMsgThread to handle the sending and receiving functions.
   - Rewrite the StopThread function: use WaitForSingleObject function now.

 - 2014.06.13 Version 0.1.1 alpha
   - Add the function of buffering Texture - significantly improves the performance. 
   - Now UpdateTexture() run once a second, not once a frame.
   - Draw the text by character, not by sentence, capable for very long sentences.
   - The characters which won't appear in 1 sec will not be drawn in UpdateTexture().
   - Use OBS default Log() instead of any file loggings.
   - Lowercase the #channel string.
   - Ignore the message from Nightbot and Moobot.
   - Recalculate the scroll speed; 10 stands for 5 seconds.
   - Fixed the port used by JTV to be 6667 (but not shown on text)
   
 - 2014.06.11 Version 0.1 alpha
   - The very first version. Still lots of bugs to fix.
   
Installation
========
Put NicoCommentPlugin.dll and NicoCommentPlugin folder into OBS's plugins folder.

Licence
========
GNU GPL v2

Download
https://mega.co.nz/#F!MBESTYoJ!vyWh9fJ45Rbdkwf-S_fIIw

Source Code
==========
Nico Comment Plugin(GitHub):
https://github.com/Appendko/NicoCommentPlugin

Author
==========
Append Huang
Append@gmail.com

Reference
==========
The text rendering functions are based on TextSource in OBS.
https://github.com/jp9000/OBS/

The basic structure of the IRCbot part is based on Fuunkao Sekai's JTChat
https://github.com/fuunkaosekai/JTChat


