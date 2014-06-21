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
#include <queue>
#include <list>
#include <string>
#include <Unknwn.h>    
#include <gdiplus.h>

#include "resource.h"
#include "constants.h"
#include "Log.h"
#include "SimpleSocket.h"
#include "SimpleThread.h"
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

extern LocaleStringLookup *pluginLocale;
#define PluginStr(text) pluginLocale->LookupString(TEXT(text))

inline DWORD GetAlphaVal(UINT opacityLevel)
{
    return ((opacityLevel*255/100)&0xFF) << 24;
}
enum MSGTYPE {MSG_USER, MSG_CHAT};

typedef struct comment {
	String comment_str;
	UINT comment_Row;
	Gdiplus::RectF boundingBox;
	UINT comment_color;
	MSGTYPE MsgType;
} Comment;


bool isFinished(const Comment &obj) { return (obj.boundingBox.GetRight()<0.0f); }

class NicoCommentPlugin : public ImageSource
{
	//Dialog Input Parameters
	//Design
    UINT        extentWidth, extentHeight;
	UINT		NumOfLines;
    int         scrollSpeed;
    DWORD       backgroundColor;
    UINT        backgroundOpacity;
    bool        bUsePointFiltering;
	//Message
    String      strFont;
    int         size;
	DWORD       color;
    UINT        opacity;
    bool        bBold, bItalic, bUnderline;
    bool        bUseOutline;
    float       outlineSize;
    DWORD       outlineColor;
    UINT        outlineOpacity;
	//Nickname
	bool		bUseNickname;
	DWORD		NicknameFallbackColor;
	bool		bUseNicknameColor;

    String      strNicknameFont;
    int         nicknameSize;
	DWORD       nicknameColor;
    UINT        nicknameOpacity;
    bool        bNicknameBold, bNicknameItalic, bNicknameUnderline;
    bool        bNicknameUseOutline;
    float       nicknameOutlineSize;
    DWORD       nicknameOutlineColor;
    UINT        nicknameOutlineOpacity;
	bool        bNicknameOutlineAutoColor;

	//Connect
	UINT iServer;
	UINT iPort;
	String Nickname;
	String Password;
	String Channel;
	bool		bUseJustinfan;

	//Internal Structure
	

	std::wstring server;
	std::wstring port;
	std::wstring nickname;
	std::wstring login;
	std::wstring password;
	std::wstring channel;
	std::wstring last_server;
	std::wstring last_port;
	std::wstring last_nickname;
	std::wstring last_login;
	std::wstring last_password;
	std::wstring last_channel;
	std::wstring justinfan;

	std::list<Comment> *Lcomment;
	UINT		Row[10];
	IRCBot		ircbot;

    bool        bDoUpdate;
	bool        bUpdateTexture;

	float		scrollValue;
	float		duration;
	float		ircTimer;
    float       showExtentTime;

    Vect2       baseSize;
    SIZE        textureSize;
	Texture     *texture;

    SamplerState *ss;
    std::unique_ptr<SamplerState> sampler;
    UINT        globalOpacity;

    XElement    *data;

	UINT Rand_Height_Generator(){
		//random number generator for Height
		UINT rand_num;
		rand_s(&rand_num);
		//detecting empty row;
		UINT iAnyZero=0;
		UINT ZeroRow[10]; 
		for(UINT i=0;i<NumOfLines;i++){
			if(Row[i]==0) {
				ZeroRow[iAnyZero]=i;
				iAnyZero++;
			}
		};

		if(iAnyZero!=0)	rand_num=ZeroRow[rand_num%iAnyZero];//rand only in these rows
		else { //weighted rand.
			UINT maxRow=0;
			UINT TotalWeight=0;
			UINT RowNextWeight[10];
			for(UINT i=0;i<NumOfLines;i++) if(Row[i]>maxRow) maxRow=Row[i];
			for(UINT i=0;i<NumOfLines;i++) {
				TotalWeight+=(maxRow-Row[i]);
				RowNextWeight[i]=TotalWeight;
			}
			rand_num%=TotalWeight;
			for(UINT i=0;i<NumOfLines;i++) {
				if (rand_num<RowNextWeight[i]){
					rand_num=i;
					break;
				}
			}
		}
		return rand_num;
	}

    void DrawOutlineText(Gdiplus::Graphics *graphics,
                         Gdiplus::Font &font,
                         const Gdiplus::GraphicsPath &path,
                         const Gdiplus::StringFormat &format,
                         const Gdiplus::Brush *brush,
						 MSGTYPE MsgType, DWORD SpecifiedOutlineColor)
    {
                
        Gdiplus::GraphicsPath *outlinePath;

        outlinePath = path.Clone();

        // Outline color and size //Try for independent opacity
		Gdiplus::Pen* pen;
        switch(MsgType) {
			case MSG_USER:
				 pen=new Gdiplus::Pen(Gdiplus::Color(GetAlphaVal((UINT)nicknameOutlineOpacity) | (SpecifiedOutlineColor&0x00FFFFFF)), nicknameOutlineSize);
				break;
			case MSG_CHAT:
				 pen=new Gdiplus::Pen(Gdiplus::Color(GetAlphaVal((UINT)outlineOpacity) | (SpecifiedOutlineColor&0x00FFFFFF)), outlineSize);
				break;
		}

        pen->SetLineJoin(Gdiplus::LineJoinRound);

        // Draw the outline
        graphics->DrawPath(pen, outlinePath);

        // Draw the text        
        graphics->FillPath(brush, &path);

		delete pen;
        delete outlinePath;
    }

    HFONT GetFont(MSGTYPE MsgType)
    {
        HFONT hFont = NULL;

        LOGFONT lf;
        zero(&lf, sizeof(lf));
        lf.lfQuality = ANTIALIASED_QUALITY;
		String strChooseFont;

		switch (MsgType) {
			case MSG_USER:
			lf.lfHeight = nicknameSize;
			lf.lfWeight = bNicknameBold ? FW_BOLD : FW_DONTCARE;
			lf.lfItalic = bNicknameItalic;
			lf.lfUnderline = bNicknameUnderline;
			strChooseFont=strNicknameFont;
			break;

			case MSG_CHAT:
			lf.lfHeight = size;
			lf.lfWeight = bBold ? FW_BOLD : FW_DONTCARE;
			lf.lfItalic = bItalic;
			lf.lfUnderline = bUnderline;
			strChooseFont=strFont;
			break;
		}

		if(strChooseFont.IsValid())
		{
	        scpy_n(lf.lfFaceName, strChooseFont, 31);
			hFont = CreateFontIndirect(&lf);
		}

        if(!hFont)
        {
            scpy_n(lf.lfFaceName, TEXT("Arial"), 31);
            hFont = CreateFontIndirect(&lf);
        }

        return hFont;
    }
/*	HFONT GetFallbackFont() //Unicode Fallback Font: Arial Unicode MS from MS OFFICE
    {
        HFONT hFont = NULL;
        LOGFONT lf;
        zero(&lf, sizeof(lf));
        lf.lfHeight = size;
        lf.lfWeight = bBold ? FW_BOLD : FW_DONTCARE;
        lf.lfItalic = bItalic;
        lf.lfUnderline = bUnderline;
        lf.lfQuality = ANTIALIASED_QUALITY;
		scpy_n(lf.lfFaceName, TEXT("Arial Unicode MS"), 31);
        hFont = CreateFontIndirect(&lf);
        if(!hFont) { //if Arial Unicode MS not found
            hFont = GetFont(); //use USER-Selected Font
        }
        return hFont;
    }*/

    void SetStringFormat(Gdiplus::StringFormat &format)
    {
        UINT formatFlags;

        formatFlags = Gdiplus::StringFormatFlagsNoFitBlackBox
                    | Gdiplus::StringFormatFlagsMeasureTrailingSpaces;

        format.SetFormatFlags(formatFlags);
        format.SetTrimming(Gdiplus::StringTrimmingWord);
    }

	float Calculate_BoundaryBox(String msg, Gdiplus::PointF origin, Gdiplus::RectF &boundingBox, MSGTYPE MsgType )
	{
		HDC hdc = CreateCompatibleDC(NULL);
		HFONT hFont=GetFont(MsgType); if(!hFont) return 0.0f;
		Gdiplus::Font font(hdc, hFont);
		//SelectObject(hdc,hFont);
		//WORD GlyInd='\0';
		//int iError=GetGlyphIndicesW(hdc, msg.Array(), 1, &GlyInd, GGI_MARK_NONEXISTING_GLYPHS);
		//if(GlyInd==0xFFFF) {
		//	onDebugMsg(L"GetGlyphIndicesW: msg=%ls, Glyind=0x%04X, Use Fallback Font instead",msg.Array(),GlyInd);
		//	hFont=GetFallbackFont();
		//	SelectObject(hdc,hFont);
		//	iError=GetGlyphIndicesW(hdc, msg.Array(), 1, &GlyInd, GGI_MARK_NONEXISTING_GLYPHS);
		//	onDebugMsg(L"Using FallbackFont, : msg=%ls, Glyind=0x%04X", msg.Array(), GlyInd);
		//}
		//ABCFLOAT tmpABCFloat;
		//GetCharABCWidthsFloat(hdc, GlyInd, GlyInd, &tmpABCFloat);
		//onDebugMsg(L"Font ABC: msg=%ls, A: %f, B:%f, C:%f, A+B+C:%f", msg.Array(), 
		//	tmpABCFloat.abcfA, tmpABCFloat.abcfB, tmpABCFloat.abcfC, 
		//	tmpABCFloat.abcfA+tmpABCFloat.abcfB+tmpABCFloat.abcfC );

		Gdiplus::Graphics *graphics = new Gdiplus::Graphics(hdc);
		Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());
		SetStringFormat(format);
		graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		
		Gdiplus::Status stat = graphics->MeasureString(msg, -1, &font, origin, &format, &boundingBox);
		//onDebugMsg(L"Font Width: msg=%ls, BoxWidth:%f, Diff:%f", msg.Array(), boundingBox.Width,
		//	boundingBox.Width-(tmpABCFloat.abcfA+tmpABCFloat.abcfB+tmpABCFloat.abcfC) );
		if(stat != Gdiplus::Ok)
			AppWarning(TEXT("NicoCommentPlugin::UpdateTexture: Gdiplus::Graphics::MeasureString failed: %u %ls"), (int)stat,msg.Array());	
		delete graphics;	
		DeleteDC(hdc);
		hdc = NULL;
		DeleteObject(hFont);		
		//TEST
		//boundingBox.Width=(tmpABCFloat.abcfB);
		//return (float)(boundingBox.X+tmpABCFloat.abcfA+tmpABCFloat.abcfB+tmpABCFloat.abcfC);
		return boundingBox.GetRight();

	}

	float PushMsgIntoList(std::wstring msg, UINT rand_num, UINT iColor, float tmpX, MSGTYPE MsgType ) {
		UINT msglen=(UINT)msg.length();
		if(msglen>0){
			//Parse into iNumChars characters....need tuning, maybe 10 or higher
			std::queue<std::wstring> Qtmp;
			for(UINT i=0;i<((msglen-1)/ iNumChars );i++) Qtmp.push(msg.substr(i*iNumChars,iNumChars));
			Qtmp.push(msg.substr(((msglen-1)/iNumChars)*iNumChars));
			//push into Lcomment
			while(!Qtmp.empty())
				{
				Comment tmp_comment;
				tmp_comment.comment_str=String(Qtmp.front().c_str());
				tmp_comment.comment_Row=rand_num;
				tmp_comment.comment_color=iColor;
				tmp_comment.MsgType=MsgType;
				Row[tmp_comment.comment_Row]++;
				tmp_comment.boundingBox=Gdiplus::RectF(0.0f,0.0f,32.0f,32.0f);
				Gdiplus::PointF origin(tmpX,float(rand_num)/float(NumOfLines)*(float)extentHeight);
				//MeasureString: calculate the corresponding bounding Box;
				tmpX=Calculate_BoundaryBox(tmp_comment.comment_str, origin, tmp_comment.boundingBox, MsgType);
				Lcomment->push_back(tmp_comment);
				Qtmp.pop();
			}
		}
		return tmpX;
	}


    void UpdateTexture()
    {
		//Log(TEXT("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d."),Row[0],Row[1],Row[2],Row[3],Row[4],Row[5],Row[6],Row[7],Row[8],Row[9]);
        HFONT hChatFont = GetFont(MSG_CHAT); if(!hChatFont) return;
		HFONT hUserFont = GetFont(MSG_USER); if(!hUserFont) return;

//		HFONT hFallbackFont = GetFallbackFont(); 
        Gdiplus::Status stat;
        Gdiplus::RectF layoutBox;
        
        HDC hTempDC = CreateCompatibleDC(NULL);
		Gdiplus::Font chatFont(hTempDC, hChatFont);  
		Gdiplus::Font userFont(hTempDC, hUserFont);  
/*		Gdiplus::Font fallbackfont(hTempDC, hFallbackFont);*/

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
        

		Gdiplus::GraphicsPath path;
		Gdiplus::FontFamily fontFamily;
		Gdiplus::Font* font;

		for (std::list<Comment>::iterator it = Lcomment->begin(); it != Lcomment->end(); it++) {
			//Check For Fallback;
/*			SelectObject(hTempDC,hFont);
			WORD GlyInd='\0';
			GetGlyphIndicesW(hTempDC,it->comment_str.Array(),1,&GlyInd,GGI_MARK_NONEXISTING_GLYPHS);
			if(GlyInd==0xFFFF) {
				font=&fallbackfont;
				font->GetFamily(&fontFamily);
			}
			else {		
				font=&userFont;
				font->GetFamily(&fontFamily);
			}*/
			bool bCommentUseOutline = false;
			DWORD tmp_OutlineColor=outlineColor;
			switch(it->MsgType){
				case MSG_USER:
					font=&userFont;
					font->GetFamily(&fontFamily);
					bCommentUseOutline=bNicknameUseOutline;
					if(bNicknameOutlineAutoColor&&bUseNicknameColor){ 
						UINT brightness=((it->comment_color&0x00FF0000)>>16) + ((it->comment_color&0x0000FF00)>>8) + (it->comment_color&0x000000FF);
						if( brightness > 3*128 ) tmp_OutlineColor=0xFF000000;//(black line)
						else tmp_OutlineColor=0xFFFFFFFF;
					} else tmp_OutlineColor=nicknameOutlineColor;

					break;
				case MSG_CHAT:
					font=&chatFont;
					font->GetFamily(&fontFamily);
					bCommentUseOutline=bUseOutline;
					tmp_OutlineColor=outlineColor;
					break;
			}
			//Draw The Character One By One
			if(it->boundingBox.GetLeft()<TextureWidth) {		
				if(bCommentUseOutline) {
					brush->SetColor(Gdiplus::Color(GetAlphaVal(opacity)|(it->comment_color&0x00FFFFFF)));
					path.Reset();
					path.AddString(it->comment_str, -1, &fontFamily, font->GetStyle(), font->GetSize(), it->boundingBox, &format);
					DrawOutlineText(graphics, *font, path, format, brush, it->MsgType,tmp_OutlineColor);
				} else {
					brush->SetColor(Gdiplus::Color(GetAlphaVal(opacity)|(it->comment_color&0x00FFFFFF)));
					stat = graphics->DrawString(it->comment_str, -1, font, it->boundingBox, &format, brush);
					if(stat != Gdiplus::Ok)
	                    AppWarning(TEXT("TextSource::UpdateTexture: Graphics::DrawString failed: %u"), (int)stat);
				}
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
            AppWarning(TEXT("NicoCommentPlugin::UpdateTexture: could not create texture"));
            DeleteObject(hChatFont);
			DeleteObject(hUserFont);
        }
		
        DeleteDC(hTempDC);
        DeleteObject(hBitmap);
		scrollValue=0.0f;
    }

public:
    inline NicoCommentPlugin(XElement *data)
    {
		ircTimer = 4.0f;
		last_server=std::wstring(L"");
		last_port=std::wstring(L"");
		last_nickname=std::wstring(L"");
		last_login=std::wstring(L"");
		last_password=std::wstring(L"");
		last_channel=std::wstring(L"");
		justinfan=std::wstring(L"");
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
		//last_row1 = 0;
		//last_row2 = 20;
		Lcomment = new std::list<Comment>;
		duration = 0.0f;

        onSysMsg(L"Using Nico Comment Plugin Version %d.%d.%d.%d Alpha",PLUGIN_VERSION);
		zero(&Row,sizeof(UINT)*10);
		login=L"NICO_COMMENT_PLUGIN_ALPHA"; //not used

    }

    ~NicoCommentPlugin()
    {
		onSysMsg(L"Closing NicoComment Plugin-Begin Process");
		ircbot.close();
		onSysMsg(L"Closing NicoComment Plugin-IRCBot Closed");
        if(texture)
        {
            delete texture;
            texture = NULL;
        }
        delete ss;
		delete Lcomment;
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
		ircTimer+=fSeconds;
		if(ircTimer > 5.0f) {
			ircTimer=0.0f;
			IRCBotStatus iStatus=ircbot.CheckIRCBotStatus();
			if(iStatus==BOT_CONNECTED) iStatus=ircbot.AliveCheckTask();
			if(iStatus==BOT_NEEDRECONNECT) ircbot.reconnect();
		}

        if(scrollSpeed != 0 && texture)
            scrollValue += fSeconds*fMsgSpeed/((float)(extentWidth)+(fMsgBufferTime*fMsgSpeed*fRatio));
		
		if(!Lcomment->empty()) {
			for (std::list<Comment>::iterator it = Lcomment->begin(); it != Lcomment->end(); it++) {
				it->	boundingBox.X-=fSeconds*fMsgSpeed; 
				if(it->comment_Row!=99)if(it->boundingBox.GetRight() < extentWidth){ //99 for removed
					Row[it->comment_Row]--;
					it->comment_Row=99;
				}
			}
			Lcomment->remove_if(isFinished);//REMOVE object at the left of the screen
			if( Lcomment->empty() ) bDoUpdate = true; //Empty out everything: need to clear texture.
		}

        //if(showExtentTime > 0.0f) showExtentTime -= fSeconds;
		//Not used; maybe add back.
		float fMsgBegin=(float)extentWidth+fMsgBufferTime*fMsgSpeed;
		float tmpX=fMsgBegin;
		TircMsg tircmsg;
		std::wstring msg;
		while(!ircbot.QueueEmpty()){
			//receive Msg 
			ircbot.receiveMsg(tircmsg);
			UINT rand_num=Rand_Height_Generator();
			if(bUseNickname) {
				UINT iColor=NicknameFallbackColor;
				if(bUseNicknameColor) if(tircmsg.usercolor!=0) iColor=tircmsg.usercolor;
				tmpX=PushMsgIntoList(tircmsg.user+L": ", rand_num, iColor, tmpX, MSG_USER);
			}
			tmpX=PushMsgIntoList(tircmsg.msg, rand_num, color, tmpX, MSG_CHAT);
		}

		//Update Texture after 1 seconds.
		if(Lcomment->empty()){
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
    {
		baseSize.x  = data->GetFloat(TEXT("baseSizeCX"), 100);
        baseSize.y  = data->GetFloat(TEXT("baseSizeCY"), 100);
		//Design
        extentWidth = data->GetInt(TEXT("extentWidth"), 0);
        extentHeight= data->GetInt(TEXT("extentHeight"), 0);
		NumOfLines = data->GetInt(TEXT("NumOfLines"), 0);
        scrollSpeed = data->GetInt(TEXT("scrollSpeed"), 0);
        backgroundColor   = data->GetInt(TEXT("backgroundColor"), 0xFF000000);
        backgroundOpacity = data->GetInt(TEXT("backgroundOpacity"), 0);
        bUsePointFiltering = data->GetInt(TEXT("pointFiltering"), 0) != 0;

		//Message
        strFont     = data->GetString(TEXT("font"), TEXT("Arial"));
        size        = data->GetInt(TEXT("fontSize"), 48);
		color       = data->GetInt(TEXT("color"), 0xFFFFFFFF);
        opacity     = data->GetInt(TEXT("textOpacity"), 100);
        bBold       = data->GetInt(TEXT("bold"), 0) != 0;
        bItalic     = data->GetInt(TEXT("italic"), 0) != 0;
        bUnderline  = data->GetInt(TEXT("underline"), 0) != 0;
        bUseOutline    = data->GetInt(TEXT("useOutline")) != 0;
        outlineColor   = data->GetInt(TEXT("outlineColor"), 0xFF000000);
        outlineSize    = data->GetFloat(TEXT("outlineSize"), 2);
        outlineOpacity = data->GetInt(TEXT("outlineOpacity"), 100);

		//Nickname
		bUseNickname = data->GetInt(TEXT("useNickname"), 0) != 0;
		NicknameFallbackColor = data->GetInt(TEXT("NicknameFallbackColor"), 0) ;
		bUseNicknameColor = data->GetInt(TEXT("useNicknameColor"), 0) != 0;
        strNicknameFont			= data->GetString(TEXT("nicknameFont"), TEXT("Arial"));
        nicknameSize			= data->GetInt(TEXT("nicknameSize"), 48);
		nicknameColor			= data->GetInt(TEXT("nicknameColor"), 0xFFFFFFFF);
        nicknameOpacity			= data->GetInt(TEXT("nicknameOpacity"), 100);
        bNicknameBold			= data->GetInt(TEXT("nicknameBold"), 0) != 0;
        bNicknameItalic			= data->GetInt(TEXT("nicknameItalic"), 0) != 0;
        bNicknameUnderline		= data->GetInt(TEXT("nicknameUnderline"), 0) != 0;
        bNicknameUseOutline		= data->GetInt(TEXT("nicknameUseOutline")) != 0;
        nicknameOutlineColor	= data->GetInt(TEXT("nicknameOutlineColor"), 0xFF000000);
        nicknameOutlineSize		= data->GetFloat(TEXT("nicknameOutlineSize"), 2);
        nicknameOutlineOpacity	= data->GetInt(TEXT("nicknameOutlineOpacity"), 100);
        bNicknameOutlineAutoColor	= data->GetInt(TEXT("nicknameOutlineAutoColor")) != 0;

		//Connection
		iServer = data->GetInt(TEXT("iServer"), 0);
		iPort = data->GetInt(TEXT("iPort"), 0);
		Nickname    = data->GetString(TEXT("Nickname"));
        Password    = data->GetString(TEXT("Password"));
		Channel		= data->GetString(TEXT("Channel"));
		bUseJustinfan = data->GetInt(TEXT("useJustinfan"),1) != 0;

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
		else if(scmpi(lpName, TEXT("nicknameFont")) == 0)
            strNicknameFont = lpVal;

        bUpdateTexture = true;
    }

    void SetInt(CTSTR lpName, int iValue)
    {
        if(scmpi(lpName, TEXT("extentWidth")) == 0) {            
            extentWidth = iValue; //showExtentTime = 2.0f;
			return;
        }
        else if(scmpi(lpName, TEXT("extentHeight")) == 0) {
            extentHeight = iValue; //showExtentTime = 2.0f;
			return;
        }
		
        if(scmpi(lpName, TEXT("NumOfLines")) == 0)
            NumOfLines = iValue;
        else if(scmpi(lpName, TEXT("scrollSpeed")) == 0)
            scrollSpeed = iValue;
        else if(scmpi(lpName, TEXT("backgroundColor")) == 0)
            backgroundColor = iValue;
        else if(scmpi(lpName, TEXT("backgroundOpacity")) == 0)
            backgroundOpacity = iValue;
		else if(scmpi(lpName, TEXT("pointFiltering")) == 0)
			bUsePointFiltering = iValue != 0;
		//----------------------------------------------------------------
		else if(scmpi(lpName, TEXT("fontSize")) == 0)
            size = iValue;
        else if(scmpi(lpName, TEXT("color")) == 0)
            color = iValue;
        else if(scmpi(lpName, TEXT("textOpacity")) == 0)
            opacity = iValue;
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
		//----------------------------------------------------------------
		else if(scmpi(lpName, TEXT("useNickname")) == 0)
            bUseNickname = iValue != 0;
        else if(scmpi(lpName, TEXT("NicknameFallbackColor")) == 0)
            NicknameFallbackColor = iValue;
		else if(scmpi(lpName, TEXT("useNicknameColor")) == 0)
            bUseNicknameColor = iValue != 0;
		else if(scmpi(lpName, TEXT("nicknameSize")) == 0)
            nicknameSize = iValue;
        else if(scmpi(lpName, TEXT("nicknameColor")) == 0)
            nicknameColor = iValue;
        else if(scmpi(lpName, TEXT("nicknameOpacity")) == 0)
            nicknameOpacity = iValue;
        else if(scmpi(lpName, TEXT("nicknameBold")) == 0)
            bNicknameBold = iValue != 0;
        else if(scmpi(lpName, TEXT("nicknameItalic")) == 0)
            bNicknameItalic = iValue != 0;
        else if(scmpi(lpName, TEXT("nicknameUnderline")) == 0)
            bNicknameUnderline = iValue != 0;
        else if(scmpi(lpName, TEXT("nicknameUseOutline")) == 0)
            bNicknameUseOutline = iValue != 0;
        else if(scmpi(lpName, TEXT("nicknameOutlineColor")) == 0)
            nicknameOutlineColor = iValue;
        else if(scmpi(lpName, TEXT("nicknameOutlineOpacity")) == 0)
            nicknameOutlineOpacity = iValue;
        else if(scmpi(lpName, TEXT("nicknameOutlineAutoColor")) == 0)
            bNicknameOutlineAutoColor = iValue != 0;
		//----------------------------------------------------------------
	    else if(scmpi(lpName, TEXT("iServer")) == 0)
			{iServer = iValue; TryConnect();}
		else if(scmpi(lpName, TEXT("iPort")) == 0)
			{iPort = iValue; TryConnect();}
		else if(scmpi(lpName, TEXT("useJustinfan")) == 0)
			{bUseJustinfan = iValue != 0; TryConnect();}


        bUpdateTexture = true;
    }

    void SetFloat(CTSTR lpName, float fValue)
    {
        if(scmpi(lpName, TEXT("outlineSize")) == 0)
            outlineSize = fValue;
        else if(scmpi(lpName, TEXT("nicknameOutlineSize")) == 0)
            nicknameOutlineSize = fValue;
		else if(scmpi(lpName, TEXT("baseSizeCX")) == 0)
            baseSize.x = fValue;
		else if(scmpi(lpName, TEXT("baseSizeCY")) == 0)
            baseSize.y = fValue;

        bUpdateTexture = true;
    }

	void TryConnect()
	{
		login=std::wstring(L"NICO_COMMENT_PLUGIN_ALPHA"); //not used
		switch(iServer){
			case 0: 	{server=std::wstring(L"irc.twitch.tv");break;}
			case 1: 	{server=std::wstring(L"irc.justin.tv");break;}
			default: 	{server=std::wstring(L"");break;}
		}

		switch(iPort){			
			case 0: 	{port=std::wstring(L"443");break;}
			case 1: 	{port=std::wstring(L"6667");break;}
			case 2: 	{port=std::wstring(L"80");break;}
			default: 	{port=std::wstring(L"");break;}
		}

		if(bUseJustinfan){
			if(justinfan.empty()) { //generate a new justinfan name
				wchar_t buffer[20];
				UINT rand_num;
				rand_s(&rand_num);
				swprintf_s(buffer,20*sizeof(wchar_t),L"justinfan%06d",rand_num%1000000);
				justinfan=std::wstring(buffer);
			}
			//Apply the nick/pass
			nickname=justinfan;
			password=std::wstring(L"blah");
		}
		else {
			if(!Nickname.IsEmpty()) nickname=std::wstring(Nickname.Array());
			else nickname=std::wstring(L"");
			if(!Password.IsEmpty()) password=std::wstring(Password.Array());
			else password=std::wstring(L"");
		}

		if(!Channel.IsEmpty()) 	{
			Channel=Channel.MakeLower();
			if(Channel.Array()[0]!=L'#') channel=std::wstring(L"#")+std::wstring(Channel.Array());
			else channel=std::wstring(Channel.Array());
		}
		else channel=L"";

		if(ircbot.isConnected()) { //Check if disconnection needed
			if(	last_server.compare(server)!=0 || last_port.compare(port)!=0 ||	
				last_nickname.compare(nickname)!=0 || last_password.compare(password)!=0 ||	
				last_channel.compare(channel)!=0 ) { //any one of them differs: Change of detail, need reconnect
					ircbot.close();
					//Sleep(500);
				}
		}

		if(!ircbot.isConnected()) { //if not connected, connect at first - If enough information
			if(server.empty()||port.empty()||nickname.empty()||password.empty()||channel.empty())
				onSysMsg(L"Not Enough Login Information. ");
			else{
				last_server=server;  last_port=port;  last_nickname=nickname;
				last_password=password;  last_channel=channel;
				ircbot.connect(last_server,last_port,last_nickname,login,last_password,last_channel);
			}
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

