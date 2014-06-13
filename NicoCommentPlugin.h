/********************************************************************************
 Copyright (C) 2014 Append Huang <Append@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/

#define _CRT_RAND_S
#include <stdlib.h>

#include "OBSApi.h"
#include <memory>
#include <vector>
#include <string>
#include <Unknwn.h>    
#include <gdiplus.h>

#include "resource.h"
#include "constants.h"
#include "Log.h"
#include "SimpleSocket.h"
#include "SimpleThread.h"
#include "SimpleTimer.h"
#include "SendReceiveThread.h"
#include "IRCMsgThread.h"
#include "IRCBot.h"

// Message Queued for some time: 
// The Frequency of UpdateTexture
#define fMsgBufferTime 1.0f
#define fMsgSpeed (float)(scrollSpeed*extentWidth) / 50.0f
#define fRatio 1.01f
#define iNumChars 1

#define ClampVal(val, minVal, maxVal) \
    if(val < minVal) val = minVal; \
    else if(val > maxVal) val = maxVal;

/*const wchar_t* LogFileName = L"Log.txt";
const wchar_t* SysFileName = L"Sys.txt";
const wchar_t* DebugFileName = L"Debug.txt";
const wchar_t* IRCFileName = L"IRC.txt";*/

inline DWORD GetAlphaVal(UINT opacityLevel)
{
    return ((opacityLevel*255/100)&0xFF) << 24;
}

typedef struct comment {
	String comment_str;
	Gdiplus::RectF boundingBox;
} Comment;
//typedef struct comment {	String comment_str;	Gdiplus::RectF boundingBox;	float last_time;} Comment;

class NicoCommentPlugin : public ImageSource
{
	std::wstring server;
	std::wstring port;
	std::wstring nickname;
	std::wstring login;
	std::wstring password;
	std::wstring channel;

	String Nickname;
	String Password;
	String Channel;
	UINT iServer;
	UINT iPort;

    std::vector<Comment> *Vcomment;
	UINT		last_row1, last_row2;

	IRCBot		ircbot;
	bool        bUpdateTexture;
	float		scrollValue;
	float		duration;

    Texture     *texture;
    float       showExtentTime;

    String      strFont;
    int         size;
    DWORD       color;
    UINT        opacity;
    UINT        globalOpacity;
    int         scrollSpeed;
    bool        bBold, bItalic, bUnderline;

    UINT        backgroundOpacity;
    DWORD       backgroundColor;

    bool        bUseOutline;
    float       outlineSize;
    DWORD       outlineColor;
    UINT        outlineOpacity;

    UINT        extentWidth, extentHeight;
	UINT		NumOfLines;

    Vect2       baseSize;
    SIZE        textureSize;
    bool        bUsePointFiltering;

    std::unique_ptr<SamplerState> sampler;

    bool        bDoUpdate;

    SamplerState *ss;

    XElement    *data;

    void DrawOutlineText(Gdiplus::Graphics *graphics,
                         Gdiplus::Font &font,
                         const Gdiplus::GraphicsPath &path,
                         const Gdiplus::StringFormat &format,
                         const Gdiplus::Brush *brush)
    {
                
        Gdiplus::GraphicsPath *outlinePath;

        outlinePath = path.Clone();

        // Outline color and size
        UINT tmpOpacity = (UINT)((((float)opacity * 0.01f) * ((float)outlineOpacity * 0.01f)) * 100.0f);
        Gdiplus::Pen pen(Gdiplus::Color(GetAlphaVal(tmpOpacity) | (outlineColor&0xFFFFFF)), outlineSize);
        pen.SetLineJoin(Gdiplus::LineJoinRound);

        // Widen the outline
        // It seems that Widen has a huge performance impact on DrawPath call, screw it! We're talking about freaking seconds in some extreme cases...
        //outlinePath->Widen(&pen);

        // Draw the outline
        graphics->DrawPath(&pen, outlinePath);

        // Draw the text        
        graphics->FillPath(brush, &path);

        delete outlinePath;
    }

    HFONT GetFont()
    {
        HFONT hFont = NULL;

        LOGFONT lf;
        zero(&lf, sizeof(lf));
        lf.lfHeight = size;
        lf.lfWeight = bBold ? FW_BOLD : FW_DONTCARE;
        lf.lfItalic = bItalic;
        lf.lfUnderline = bUnderline;
        lf.lfQuality = ANTIALIASED_QUALITY;

        if(strFont.IsValid())
        {
            scpy_n(lf.lfFaceName, strFont, 31);

            hFont = CreateFontIndirect(&lf);
        }

        if(!hFont)
        {
            scpy_n(lf.lfFaceName, TEXT("Arial"), 31);
            hFont = CreateFontIndirect(&lf);
        }

        return hFont;
    }

    void SetStringFormat(Gdiplus::StringFormat &format)
    {
        UINT formatFlags;

        formatFlags = Gdiplus::StringFormatFlagsNoFitBlackBox
                    | Gdiplus::StringFormatFlagsMeasureTrailingSpaces;

        format.SetFormatFlags(formatFlags);
        format.SetTrimming(Gdiplus::StringTrimmingWord);
    }

	void Calculate_BoundaryBox(String msg, Gdiplus::PointF origin, Gdiplus::RectF &boundingBox )
	{
		HDC hdc = CreateCompatibleDC(NULL);
		HFONT hFont=GetFont(); if(!hFont) return;
		Gdiplus::Font font(hdc, hFont);
		Gdiplus::Graphics *graphics = new Gdiplus::Graphics(hdc);
		Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());
		SetStringFormat(format);
		graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		
		Gdiplus::Status stat = graphics->MeasureString(msg, -1, &font, origin, &format, &boundingBox);
		if(stat != Gdiplus::Ok)
		    AppWarning(TEXT("TextSource::UpdateTexture: Gdiplus::Graphics::MeasureString failed: %u"), (int)stat);
		//if(bUseOutline)
		//	boundingBox.Offset(outlineSize/2, outlineSize/2); 
		
		delete graphics;	
		DeleteDC(hdc);
		hdc = NULL;
		DeleteObject(hFont);		
	}

    void UpdateTexture()
    {
        HFONT hFont = GetFont(); if(!hFont) return;
        Gdiplus::Status stat;
        Gdiplus::RectF layoutBox;
        
        HDC hTempDC = CreateCompatibleDC(NULL);
		Gdiplus::Font font(hTempDC, hFont);
        Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());
        SetStringFormat(format);

		UINT TextureWidth = extentWidth+(UINT)(fMsgBufferTime*fMsgSpeed*fRatio); //extra 0.05 for uncertainty

		//----------------------------------------------------------------------
        // write image

        BITMAPINFO bi;
        zero(&bi, sizeof(bi));

        void* lpBits;

        BITMAPINFOHEADER &bih = bi.bmiHeader;
        bih.biSize = sizeof(bih);
        bih.biBitCount = 32;
        bih.biWidth  = TextureWidth; 
        bih.biHeight = extentHeight;
        bih.biPlanes = 1;

        HBITMAP hBitmap = CreateDIBSection(hTempDC, &bi, DIB_RGB_COLORS, &lpBits, NULL, 0);
		Gdiplus::Bitmap  bmp(TextureWidth, extentHeight, 4*TextureWidth, PixelFormat32bppARGB, (BYTE*)lpBits);
        Gdiplus::Graphics *graphics = new Gdiplus::Graphics(&bmp); 
        Gdiplus::SolidBrush  *brush = new Gdiplus::SolidBrush(Gdiplus::Color(GetAlphaVal(opacity)|(color&0x00FFFFFF)));

        DWORD bkColor;

		if(backgroundOpacity == 0 && scrollSpeed !=0)
            bkColor = 1<<24 | (color&0x00FFFFFF);
        else
            bkColor = GetAlphaVal(backgroundOpacity) | (backgroundColor&0x00FFFFFF); //bUseExtents is always true //never vertical //never wrap

		stat = graphics->Clear(Gdiplus::Color( bkColor ));
		if(stat != Gdiplus::Ok)
			AppWarning(TEXT("TextSource::UpdateTexture: Graphics::Clear failed: %u"), (int)stat);

        graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
        graphics->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
        graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        

        if(bUseOutline)
        {
			Gdiplus::GraphicsPath path;
            Gdiplus::FontFamily fontFamily;
			font.GetFamily(&fontFamily);
			for(unsigned int i=0;i<Vcomment->size();i++) /*{Vcomment->at(i).boundingBox.X-=duration;}*/
				if(Vcomment->at(i).boundingBox.GetLeft()<TextureWidth) {
					path.Reset();
					path.AddString(Vcomment->at(i).comment_str, -1, &fontFamily, font.GetStyle(), font.GetSize(), Vcomment->at(i).boundingBox, &format);
					DrawOutlineText(graphics, font, path, format, brush);
				}
		}
        else
        	for(unsigned int i=0;i<Vcomment->size();i++) /*{Vcomment->at(i).boundingBox.X-=duration;}*/
			{
				if(Vcomment->at(i).boundingBox.GetLeft()<TextureWidth) {
					stat = graphics->DrawString(Vcomment->at(i).comment_str, -1, &font, Vcomment->at(i).boundingBox, &format, brush);
					if(stat != Gdiplus::Ok)
	                    AppWarning(TEXT("TextSource::UpdateTexture: Graphics::DrawString failed: %u"), (int)stat);
				}
            }
        
        delete brush;
        delete graphics;

        //----------------------------------------------------------------------
        // upload texture
		if(textureSize.cx != TextureWidth || textureSize.cy != extentHeight){
            if(texture)
                {
                    delete texture;
                    texture = NULL;
                }
			texture = CreateTexture(TextureWidth, extentHeight, GS_BGRA, lpBits, FALSE, FALSE);
			textureSize.cx=TextureWidth;
			textureSize.cy=extentHeight;
		}

        else if(texture)
            texture->SetImage(lpBits, GS_IMAGEFORMAT_BGRA, 4*TextureWidth);
		
        if(!texture)
        {
            AppWarning(TEXT("TextSource::UpdateTexture: could not create texture"));
            DeleteObject(hFont);
        }
		
        DeleteDC(hTempDC);
        DeleteObject(hBitmap);
		scrollValue=0.0f;
    }

public:
    inline NicoCommentPlugin(XElement *data)
    {
        this->data = data;
        UpdateSettings();

        SamplerInfo si;
        zero(&si, sizeof(si));
        si.addressU = GS_ADDRESS_REPEAT;
        si.addressV = GS_ADDRESS_REPEAT;
        si.borderColor = 0;
        si.filter = GS_FILTER_LINEAR;
        ss = CreateSamplerState(si);
        globalOpacity = 100;
		last_row1 = 0;
		last_row2 = 20;
		Vcomment = new std::vector<Comment>;
		duration = 0.0f;
        Log(TEXT("Using text output"));

		login=L"NICO_COMMENT_PLUGIN_ALPHA"; //not used
    }

    ~NicoCommentPlugin()
    {
		//ircbot.close();
        if(texture)
        {
            delete texture;
            texture = NULL;
        }
        delete ss;
		//if(Vcomment->size()>0) Vcomment->clear();
		delete Vcomment;
    }

    void Preprocess()
    {
        if(bUpdateTexture)
        {
            bUpdateTexture = false;
            UpdateTexture();
        }
    }

    void Tick(float fSeconds)
    {
        if(scrollSpeed != 0 && texture)
        {
            scrollValue += fSeconds*fMsgSpeed/((float)(extentWidth)+(fMsgBufferTime*fMsgSpeed*fRatio));
//            while(scrollValue > 1.0f)
//                scrollValue -= 1.0f;
        }
		
		if(!Vcomment->empty()) {
		std::vector<Comment>* V_new= new std::vector<Comment>;
			for(unsigned int i=0;i<Vcomment->size();i++) {
				Vcomment->at(i).boundingBox.X-=fSeconds*fMsgSpeed;
				if(Vcomment->at(i).boundingBox.GetRight()>0)
					V_new->push_back(Vcomment->at(i));
			}
			Vcomment->clear(); delete Vcomment;
			Vcomment=V_new;
			if( Vcomment->empty() ) bDoUpdate = true; //Empty out everything: need to clear texture.
		}

        if(showExtentTime > 0.0f)
            showExtentTime -= fSeconds;

		std::wstring msg;
		while(!ircbot.QueueEmpty()){
			//receive Msg 
			ircbot.receiveMsg(msg);
			//if(msg[0]==L'!') break;
			//random number generator for Height
			UINT rand_num;
			while(true){
				rand_s(&rand_num);
				rand_num%=NumOfLines;
				if ((rand_num!=last_row1) && (rand_num!=last_row2)){
					last_row2=last_row1;
					last_row1=rand_num;
					break;
				}
			}
			//Parse into iNumChars characters....need tuning, maybe 10 or higher
			int msglen=msg.length();
			if(msglen>0){
				std::vector<std::wstring> Qtmp;
				for(int i=0;i<((msglen-1)/ iNumChars );i++) Qtmp.push_back(msg.substr(i*iNumChars,iNumChars));
				Qtmp.push_back(msg.substr(((msglen-1)/iNumChars)*iNumChars));
				//push into Qcomment
				float fMsgBegin=(float)extentWidth+fMsgBufferTime*fMsgSpeed;
				float tmpX=fMsgBegin;
				for(unsigned int i=0;i<Qtmp.size();i++)
					{
					Comment tmp_comment;
					tmp_comment.comment_str=String(Qtmp[i].c_str());
					tmp_comment.boundingBox=Gdiplus::RectF(0.0f,0.0f,32.0f,32.0f);
					Gdiplus::PointF origin(tmpX,float(rand_num)/float(NumOfLines)*(float)extentHeight);
					//MeasureString: calculate the corresponding bounding Box;
					Calculate_BoundaryBox(tmp_comment.comment_str, origin, tmp_comment.boundingBox );
					tmpX=tmp_comment.boundingBox.GetRight();
					Vcomment->push_back(tmp_comment);
					}
				Qtmp.clear();
			}
		}
		//Update Texture after 2 seconds.
		if(Vcomment->empty()){
			duration=0.0f;
			if(!bDoUpdate) {
				scrollValue=0.0f; //Empty Queue in, Empty Queue out.
				return;
			}
		}

		duration+=fSeconds;
		if(duration > (fMsgBufferTime - TINY_EPSILON)) {
			bDoUpdate = true;
			duration=0.0f;
		}

        if(bDoUpdate)
        {
            bDoUpdate = false;
            bUpdateTexture = true;
        }

    }

    void Render(const Vect2 &pos, const Vect2 &size)
    {
        if(texture)
        {
            Vect2 sizeMultiplier = size/baseSize;
            Vect2 newSize = Vect2(float(textureSize.cx), float(textureSize.cy))*sizeMultiplier;

            Vect2 extentVal = Vect2(float(extentWidth), float(extentHeight))*sizeMultiplier;
            if(showExtentTime > 0.0f)
            {
                Shader *pShader = GS->GetCurrentPixelShader();
                Shader *vShader = GS->GetCurrentVertexShader();

                Color4 rectangleColor = Color4(0.0f, 1.0f, 0.0f, 1.0f);
                if(showExtentTime < 1.0f)
                    rectangleColor.w = showExtentTime;

                //App->solidPixelShader->SetColor(App->solidPixelShader->GetParameter(0), rectangleColor);
                //LoadVertexShader(App->solidVertexShader);
                //LoadPixelShader(App->solidPixelShader);
                DrawBox(pos, extentVal);

                LoadVertexShader(vShader);
                LoadPixelShader(pShader);
            }

            XRect rect = {int(pos.x), int(pos.y), int(extentVal.x), int(extentVal.y)};
            SetScissorRect(&rect);
			
			if(bUsePointFiltering) {
                if (!sampler) {
                    SamplerInfo samplerinfo;
                    samplerinfo.filter = GS_FILTER_POINT;
                    std::unique_ptr<SamplerState> new_sampler(CreateSamplerState(samplerinfo));
                    sampler = std::move(new_sampler);
                }

                LoadSamplerState(sampler.get(), 0);
            }

            DWORD alpha = DWORD(double(globalOpacity)*2.55);
            DWORD outputColor = (alpha << 24) | 0xFFFFFF;

			UVCoord ul(0.0f, 0.0f);
            UVCoord lr(1.0f, 1.0f);
			ul.x += scrollValue;
            lr.x += scrollValue;

			LoadSamplerState(ss);
            DrawSpriteEx(texture, outputColor, pos.x, pos.y, pos.x+newSize.x, pos.y+newSize.y, ul.x, ul.y, lr.x, lr.y);
            //DrawSprite(texture, outputColor, pos.x, pos.y, pos.x+newSize.x, pos.y+newSize.y);

            if (bUsePointFiltering)
                LoadSamplerState(NULL, 0);

            SetScissorRect(NULL);
        }
    }

    Vect2 GetSize() const
    {
        return baseSize;
    }

    void UpdateSettings()
    {//bUseExtents is always true //never vertical //never wrap
        strFont     = data->GetString(TEXT("font"), TEXT("Arial"));
        color       = data->GetInt(TEXT("color"), 0xFFFFFFFF);
        size        = data->GetInt(TEXT("fontSize"), 48);
        opacity     = data->GetInt(TEXT("textOpacity"), 100);
        scrollSpeed = data->GetInt(TEXT("scrollSpeed"), 0);
        bBold       = data->GetInt(TEXT("bold"), 0) != 0;
        bItalic     = data->GetInt(TEXT("italic"), 0) != 0;
        bUnderline  = data->GetInt(TEXT("underline"), 0) != 0;
        extentWidth = data->GetInt(TEXT("extentWidth"), 0);
        extentHeight= data->GetInt(TEXT("extentHeight"), 0);
        bUsePointFiltering = data->GetInt(TEXT("pointFiltering"), 0) != 0;

        baseSize.x  = data->GetFloat(TEXT("baseSizeCX"), 100);
        baseSize.y  = data->GetFloat(TEXT("baseSizeCY"), 100);

        bUseOutline    = data->GetInt(TEXT("useOutline")) != 0;
        outlineColor   = data->GetInt(TEXT("outlineColor"), 0xFF000000);
        outlineSize    = data->GetFloat(TEXT("outlineSize"), 2);
        outlineOpacity = data->GetInt(TEXT("outlineOpacity"), 100);

        backgroundColor   = data->GetInt(TEXT("backgroundColor"), 0xFF000000);
        backgroundOpacity = data->GetInt(TEXT("backgroundOpacity"), 0);

		NumOfLines = data->GetInt(TEXT("NumOfLines"), 0);
		iServer = data->GetInt(TEXT("iServer"), 0);
		iPort = data->GetInt(TEXT("iPort"), 0);
		Nickname    = data->GetString(TEXT("Nickname"));
        Password    = data->GetString(TEXT("Password"));
		Channel		= data->GetString(TEXT("Channel"));
		
        bUpdateTexture = true;
		TryConnect();
    }

    void SetString(CTSTR lpName, CTSTR lpVal)
    {
		if(scmpi(lpName, TEXT("Nickname")) == 0)
			{Nickname = lpVal; TryConnect();}
        else if(scmpi(lpName, TEXT("Password")) == 0)
            {Password = lpVal; TryConnect();}
		else if(scmpi(lpName, TEXT("Channel")) == 0)
            {Channel = lpVal; TryConnect();}
		else if(scmpi(lpName, TEXT("font")) == 0)
            strFont = lpVal;

        bUpdateTexture = true;
    }

    void SetInt(CTSTR lpName, int iValue)
    {
        if(scmpi(lpName, TEXT("extentWidth")) == 0)
        {
            //showExtentTime = 2.0f;
            extentWidth = iValue;
			return;
        }
        else if(scmpi(lpName, TEXT("extentHeight")) == 0)
        {
            //showExtentTime = 2.0f;
            extentHeight = iValue;
			return;
        }

        if(scmpi(lpName, TEXT("color")) == 0)
            color = iValue;
        else if(scmpi(lpName, TEXT("fontSize")) == 0)
            size = iValue;
        else if(scmpi(lpName, TEXT("textOpacity")) == 0)
            opacity = iValue;
        else if(scmpi(lpName, TEXT("scrollSpeed")) == 0)
        {
            scrollSpeed = iValue;
        }
        else if(scmpi(lpName, TEXT("bold")) == 0)
            bBold = iValue != 0;
        else if(scmpi(lpName, TEXT("italic")) == 0)
            bItalic = iValue != 0;
        else if(scmpi(lpName, TEXT("underline")) == 0)
            bUnderline = iValue != 0;
        else if(scmpi(lpName, TEXT("useOutline")) == 0)
            bUseOutline = iValue != 0;
        else if(scmpi(lpName, TEXT("outlineColor")) == 0)
            outlineColor = iValue;
        else if(scmpi(lpName, TEXT("outlineOpacity")) == 0)
            outlineOpacity = iValue;
        else if(scmpi(lpName, TEXT("backgroundColor")) == 0)
            backgroundColor = iValue;
        else if(scmpi(lpName, TEXT("backgroundOpacity")) == 0)
            backgroundOpacity = iValue;
        else if(scmpi(lpName, TEXT("NumOfLines")) == 0)
            NumOfLines = iValue;
	    else if(scmpi(lpName, TEXT("iServer")) == 0)
			{iServer = iValue; TryConnect();}
		else if(scmpi(lpName, TEXT("iPort")) == 0)
			{iPort = iValue; TryConnect();}
		
        bUpdateTexture = true;
    }

    void SetFloat(CTSTR lpName, float fValue)
    {
        if(scmpi(lpName, TEXT("outlineSize")) == 0)
            outlineSize = fValue;

        bUpdateTexture = true;
    }

	void TryConnect()
	{
		if(ircbot.isConnected()) {ircbot.close();Sleep(500);}
		switch(iServer){
			case 0: 	{server=std::wstring(L"irc.twitch.tv");break;}
			case 1: 	{server=std::wstring(L"irc.justin.tv");break;}
			default: 	{server=std::wstring(L"");break;}
		}

		if(iServer==0) {
			switch(iPort){			
				case 0: 	{port=std::wstring(L"443");break;}
				case 1: 	{port=std::wstring(L"6667");break;}
				case 2: 	{port=std::wstring(L"80");break;}
				default: 	{port=std::wstring(L"");break;}
			}
		}
		if(iServer==1) port=std::wstring(L"6667");

		if(!Nickname.IsEmpty()) nickname=std::wstring(Nickname.Array());
		else nickname=std::wstring(L"");

		if(!Password.IsEmpty()) password=std::wstring(Password.Array());
		else password=std::wstring(L"");

		if(!Channel.IsEmpty()) 	{
			Channel=Channel.MakeLower();
			if(Channel.Array()[0]!=L'#') channel=std::wstring(L"#")+std::wstring(Channel.Array());
			else channel=std::wstring(Channel.Array());
		}
		else channel=L"";
		login=std::wstring(L"NICO_COMMENT_PLUGIN_ALPHA"); //not used
		//onDebugMsg(L"server %ls,port %ls\nnickname %ls\npassword %ls\nchannel %ls\n",server.c_str(),port.c_str(),nickname.c_str(),password.c_str(),channel.c_str());

		if(server.empty()||port.empty()||nickname.empty()||login.empty()||password.empty()||channel.empty())
			onDebugMsg(L"Not Enough Login Information. \n");
		else {
			ircbot.connect(server,port,nickname,login,password,channel);
			if(ircbot.isConnected()) onDebugMsg(L"IRCBot Connected. \n");
			else onDebugMsg(L"IRCBot Cannot Connect. \n");
		}
	}

    inline void ResetExtentRect() {showExtentTime = 0.0f;}
};

struct ConfigTextSourceInfo
{
    CTSTR lpName;
    XElement *data;
    float cx, cy;

    StringList fontNames;
    StringList fontFaces;
};

