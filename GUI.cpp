/// GUI class for SDL projects

// GUI TODO:
// 1. Colours in a "theme" file


//#include <SDL2/SDL.h>
#include "GUI.h"
#include "fontengine.h"


/// CGUIControl - Base class for controls
CGUIControl::CGUIControl(int x, int y, int width, int height, unsigned short type, const char *name, const char *text)
{
	m_rect.x = x;
	m_rect.y = y;
	m_rect.w = width;
	m_rect.h = height;
	m_type = type;
	strcpy(m_name, name);
	if (text)
		strcpy(m_text, text);
	else
		m_text[0] = 0;
}

SDL_Rect CGUIControl::GetRect() const
{
	return m_rect;
}

char *CGUIControl::GetText() const
{
	return (char *)&m_text[0];
}

void CGUIControl::SetText(const char *text)
{
	strcpy(m_text, text);
}

/// control rendering functions
void CGUIControl::DrawBGFill(const CGUIDrawContext &drawContext) const
{
	drawContext.SetDrawColour(drawContext.m_backColour); 
	SDL_RenderFillRect(drawContext.m_renderer, &m_rect);
}

void CGUIControl::DrawFrame(const CGUIDrawContext &drawContext) const
{
	drawContext.SetDrawColour(drawContext.m_foreColour); 
	SDL_RenderDrawRect(drawContext.m_renderer, &m_rect);
}

void CGUIControl::DrawLine(const CGUIDrawContext &drawContext, int x1, int y1, int x2, int y2) const
{
	SDL_RenderDrawLine(drawContext.m_renderer, x1, y1, x2, y2);
}

/// Draw text in the control
/// @param[in] x	X offset from left side of control
/// @param[in] y	Y offset from top of control
void CGUIControl::DrawText(const CGUIDrawContext &drawContext, int x, int y, const char *text) const
{
	if (drawContext.m_font && drawContext.m_renderer)
		{
		SDL_Rect rect = m_rect;
		rect.x += x;
		rect.y += y;
		drawContext.m_font->DrawText(drawContext.m_renderer, text, rect, true);
		}
}



/// CGUILabel - Text label control

CGUILabel::CGUILabel(int x, int y, int width, int height, const char *name, const char *text)
	:	CGUIControl(x, y, width, height, CT_LABEL, name, text)
{

}

void CGUILabel::Draw(const CGUIDrawContext &drawContext)
{
	// TODO : Calc vertical pos of text based on rect and text height
	DrawText(drawContext, 0, 0, m_text);
}


/// CGUIEdit - Text editcontrol

CGUIEdit::CGUIEdit(int x, int y, int width, int height, const char *name, const char *text)
	:	CGUIControl(x, y, width, height, CT_EDIT, name, text)
{

}

int CGUIEdit::OnClick(const CGUIDrawContext &drawContext, int x, int y)
{
	int retval = EditText(drawContext);
	return retval;
}

int CGUIEdit::OnSwipe(int x, int y, int dx, int dy)
{
	// TODO
	return 0;
}

void CGUIEdit::Draw(const CGUIDrawContext &drawContext)
{
	// TODO : Calc vertical pos of text based on rect and text height
	DrawBGFill(drawContext);
	DrawText(drawContext, 2, 0, m_text);
	DrawFrame(drawContext);
}

/// Popup text input
int CGUIEdit::EditText(const CGUIDrawContext &drawContext)
{
	// Virtual keyboard character map
	const char charMap[41] = "1234567890QWERTYUIOPASDFGHJKL^ZXCVBNM []";
	
	// We work with a local copy of the text value
	char text[256];
	strcpy(text, m_text);
	
	// LOad the virtual keyboard bitmaps
	SDL_Surface *imgKeys = SDL_LoadBMP("vk_medium.bmp");
	SDL_Surface *imgKeysHilite = SDL_LoadBMP("vk_medium_hilite.bmp");
	if (!imgKeys || !imgKeysHilite)
		{
		printf("Error loading keyboard bitmaps!\n");
		return 0;
		}

	SDL_Renderer *renderer = drawContext.m_renderer;
	SDL_Texture *txKeys = SDL_CreateTextureFromSurface(renderer, imgKeys);
	SDL_Texture *txKeysHilite = SDL_CreateTextureFromSurface(renderer, imgKeysHilite);
	
	// Setup the popup layout
	int parentWidth, parentHeight;
	SDL_RenderGetLogicalSize(renderer, &parentWidth, &parentHeight);

	const int MARGIN = 8;
	const int CW = 60;
	const int CH = 17;
	const int POPUPWIDTH = parentWidth - 2 * MARGIN;
	const int POPUPHEIGHT = POPUPWIDTH / 2;

	//int x = std::min(m_rect.x, parentWidth - POPUPWIDTH);
	//int y = std::min(m_rect.y, parentHeight - POPUPHEIGHT);
	int x0 = std::min(m_rect.x, parentWidth - POPUPWIDTH);
	int y0 = std::min(m_rect.y, parentHeight - POPUPHEIGHT);
	SDL_Rect popupRect;
	popupRect.x = x0; popupRect.y = y0; 
	popupRect.w = POPUPWIDTH; popupRect.h = POPUPHEIGHT;

	// Create a GUI control manager for our popup "layout"
	CGUIManager gmPopup(popupRect);
	gmPopup.m_drawContext = drawContext;
//	gmPopup.m_drawContext.m_font = smallFont;
//	gmPopup.m_drawContext.SetForeColour(255, 255, 255, 0);
	gmPopup.m_drawContext.SetBackColour(0, 0, 0, 255);

	// Create Done button
	CGUIButton *doneButton = gmPopup.AddButton(x0 + POPUPWIDTH - MARGIN - CW, y0 + MARGIN, CW, CH, "doneButton", "Done");

	SDL_Rect kbRect = popupRect;
	kbRect.x += MARGIN;
	kbRect.y += MARGIN * 2 + 32;
	kbRect.h -= MARGIN * 3 + 32;
	kbRect.w -= (MARGIN * 2);

	SDL_Event event;
	bool done = false;
	while(!done)
		{
		while (SDL_PollEvent(&event))
			{
			switch (event.type)
				{
				case SDL_KEYDOWN :
					// Check for ESC pressed
					if (SDLK_ESCAPE == event.key.keysym.sym)
						done = true;
					break;
				case SDL_MOUSEMOTION :
					//gmPopup.OnMouseMove(event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONDOWN :
					//gmPopup.OnMouseDown(event.button.x, event.button.y);
					{
					// get "local coords"
					int x = event.button.x - kbRect.x;
					int y = event.button.y - kbRect.y;
					if (x > 0 && x < kbRect.w && y > 0 && y < kbRect.h)
						{
						const int cx = kbRect.w / 10;
						const int cy = kbRect.h / 4;
						int i = (y / cy) * 10 + (x / cx);
						if (i >= 0 && i < 40)
							{
							char c = charMap[i];
							if ('^' == c)
								{
								// Backspace (delete the last character)
								int len = strlen(text);
								if (len > 0)
									text[len - 1] = 0;
								}
							else
								{
								// Append the character
								int len = strlen(text);
								text[len] = c;
								text[len+1] = 0;
								}
								
							}
							
						}
					}
					break;
				case SDL_MOUSEBUTTONUP :
					{
					CGUIControl *control = gmPopup.OnMouseUp(event.button.x, event.button.y);
					// If done button clicked, then close
					if (control == doneButton)
						done = true;
					}
					break;
				}	// end switch
			}	// wend event

		// Update labels
		char s[20];

		// Draw background and frame
		gmPopup.DrawAllControls();

		// Draw keyboard
		SDL_RenderDrawRect(renderer, &kbRect);

		SDL_Rect srcRect = { 0, 0, imgKeys->w, imgKeys->h };
		SDL_RenderCopy(renderer, txKeys, &srcRect, &kbRect);

		// Draw text
		SDL_Rect textRect = popupRect;
		textRect.x += MARGIN;
		textRect.y += MARGIN;
		textRect.h = 20;
		textRect.w -= (MARGIN * 2);
		drawContext.m_font->DrawText(renderer, text, textRect, true);

		//SDL_Flip(screen);
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
		}	// wend

	// Update the parent text control's value 
	strcpy(m_text, text);
	
	return 0;
}


/// CGUIButton - Button control

CGUIButton::CGUIButton(int x, int y, int width, int height, const char *name, const char *text)
	:	CGUIControl(x, y, width, height, CT_BUTTON, name, text)
{

}

int CGUIButton::OnClick(const CGUIDrawContext &drawContext, int x, int y)
{
	// TODO
	return 0;
}

int CGUIButton::OnSwipe(int x, int y, int dx, int dy)
{
	// TODO
	return 0;
}

void CGUIButton::Draw(const CGUIDrawContext &drawContext)
{
	// Centre text horizontally and vertically
	int h = drawContext.m_font->GetFontHeight();
	int w = drawContext.m_font->GetFontWidth();
	int textWidth = w * strlen(m_text);
	int x = m_rect.w / 2 - textWidth / 2;
	int y = m_rect.h / 2 - h / 2;
	DrawBGFill(drawContext);
	DrawText(drawContext, x, y, m_text);
	DrawFrame(drawContext);
}


/// CGUISlider - Slider control

CGUISlider::CGUISlider(int x, int y, int width, int height, const char *name, float minVal, float maxVal)
	:	CGUIControl(x, y, width, height, CT_SLIDER, name, NULL),
		m_value(0.5f),
		m_minVal(0.0f),
		m_maxVal(1.0f)
{

}

/// Popup slider editor
int CGUISlider::EditSlider(const CGUIDrawContext &drawContext)
{
	// Setup the popup layout
	SDL_Renderer *renderer = drawContext.m_renderer;
	int parentWidth, parentHeight;
	SDL_RenderGetLogicalSize(renderer, &parentWidth, &parentHeight);

	const int SLIDERWIDTH = 320;
	const int SLIDERHEIGHT = SLIDERWIDTH / 8;
	const int CW = 60;
	const int CH = 17;
	const int MARGIN = 4;
	const int POPUPWIDTH = SLIDERWIDTH + 2 * MARGIN;
	const int POPUPHEIGHT = SLIDERHEIGHT + CH + 3 * MARGIN;

	int x0 = std::min(m_rect.x, parentWidth - POPUPWIDTH);
	int y0 = std::min(m_rect.y, parentHeight - POPUPHEIGHT);
	SDL_Rect popupRect;
	popupRect.x = x0; popupRect.y = y0; 
	popupRect.w = POPUPWIDTH; popupRect.h = POPUPHEIGHT;

	// Create a GUI control manager for our popup "layout"
	CGUIManager gmPopup(popupRect);
	gmPopup.m_drawContext = drawContext;
//	gmPopup.m_drawContext.m_font = smallFont;
//	gmPopup.m_drawContext.SetForeColour(255, 255, 255, 0);
	gmPopup.m_drawContext.SetBackColour(0, 0, 0, 255);

	// create BIG slider
	int x1 = x0 + MARGIN;
	int y1 = y0 + MARGIN;
	CGUISlider *slider = gmPopup.AddSlider(x1, y1, SLIDERWIDTH, SLIDERHEIGHT,  "popupSlider");
	slider->m_value = this->m_value;
	slider->m_minVal = this->m_minVal;
	slider->m_maxVal = this->m_maxVal;

	// Calc x pos of "original value" line
	int originalX = x1 + (int)(SLIDERWIDTH * ((m_value - m_minVal) / (m_maxVal - m_minVal)));

	// create label
	char labelText[32];
	strcpy(labelText, m_name);
	y1 = y0 + POPUPHEIGHT - MARGIN - CH;
	CGUILabel *label = gmPopup.AddLabel(x1, y1, CW, CH, "label", labelText);

	// Create Close button
	CGUIButton *closeButton = gmPopup.AddButton(x0 + POPUPWIDTH - MARGIN - CW, y1, CW, CH, "closeButton", "Close");

	SDL_Event event;
	bool done = false;
	while(!done)
		{
		while (SDL_PollEvent(&event))
			{
			switch (event.type)
				{
				case SDL_KEYDOWN :
					// Check for ESC pressed
					if (SDLK_ESCAPE == event.key.keysym.sym)
						done = true;
					break;
				case SDL_MOUSEMOTION :
					gmPopup.OnMouseMove(event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONDOWN :
					gmPopup.OnMouseDown(event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONUP :
					{
					CGUIControl *control = gmPopup.OnMouseUp(event.button.x, event.button.y);
					// If close button clicked, then close
					if (control == closeButton)
						done = true;
					}
					break;
				}	// end switch
			}	// wend event

		// Update label
		sprintf(labelText, "%s = %.3f", m_name, slider->m_value);
		label->SetText(labelText);

		// Draw background, frame and controls
		gmPopup.DrawAllControls();

		// Draw original value over the control
		y1 = y0 + MARGIN + MARGIN;
		int y2 = y0 + SLIDERHEIGHT;
		SDL_Colour c = { 255, 0, 0, 0 };
		drawContext.SetDrawColour(c);
		SDL_RenderDrawLine(renderer, originalX, y1, originalX, y2);
		
		//SDL_Flip(screen);
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
		}	// wend

	// Apply the changed slider value
	// TODO "OK" and "Cancel" modes 
	this->m_value = slider->m_value;

	return 0;
}

int CGUISlider::OnClick(const CGUIDrawContext &drawContext, int x, int y)
{
	// Show popup BIG SLIDER window, unless we are already 
	// in BIG SLIDER window
	if (0 == strcmp(m_name, "popupSlider"))
		{
		float k = (float)x / m_rect.w;
		m_value = m_minVal + k * (m_maxVal - m_minVal);
		return 0;
		}
		
	int retval = EditSlider(drawContext);
	return retval;
}

int CGUISlider::OnSwipe(int x, int y, int dx, int dy)
{
	float k = (float)x / m_rect.w;
	m_value = m_minVal + k * (m_maxVal - m_minVal);
	return 0;
}

void CGUISlider::Draw(const CGUIDrawContext &drawContext)
{
	DrawBGFill(drawContext);
	DrawFrame(drawContext);
	float k = (m_value - m_minVal) / (m_maxVal - m_minVal);
	int x = m_rect.x + (int)(m_rect.w * k);
	SDL_Colour colour = { 255, 255, 255, 0 };
	drawContext.SetDrawColour(colour); 
	DrawLine(drawContext, x, m_rect.y, x, m_rect.y + m_rect.h - 1);
	DrawLine(drawContext, x - 1, m_rect.y + 2, x - 1, m_rect.y + m_rect.h - 3);
	DrawLine(drawContext, x + 1, m_rect.y + 2, x + 1, m_rect.y + m_rect.h - 3);
}



/// CGUIEnvelope - Envelope control

CGUIEnvelope::CGUIEnvelope(int x, int y, int width, int height, const char *name, const char *text)
	:	CGUIControl(x, y, width, height, CT_ENVELOPE, name, text)
{
	// Init ADSR values to sane
	m_adsr[0] = 0.0f;
	m_adsr[1] = 100.0f;
	m_adsr[2] = 0.9f;
	m_adsr[3] = 50.0f;
	m_adsr[4] = 0.7f;
	m_adsr[5] = 500.0f;
}

/// Popup ADSR editor
int CGUIEnvelope::EditADSR(const CGUIDrawContext &drawContext)
{
	// Setup the popup layout
	const int CW = 60;
	const int CH = 17;
	const int MARGIN = 4;
	const int POPUPWIDTH = 4 * CW + 5 * MARGIN;
	const int POPUPHEIGHT = 8 * CH + 6 * MARGIN;

	SDL_Renderer *renderer = drawContext.m_renderer;
	int parentWidth, parentHeight;
	SDL_RenderGetLogicalSize(renderer, &parentWidth, &parentHeight);

	int x0 = std::min(m_rect.x, parentWidth - POPUPWIDTH);
	int y0 = std::min(m_rect.y, parentHeight - POPUPHEIGHT);
	SDL_Rect popupRect;
	popupRect.x = x0; popupRect.y = y0; 
	popupRect.w = POPUPWIDTH; popupRect.h = POPUPHEIGHT;

	// Create a GUI control manager for our popup "layout"
	CGUIManager gmPopup(popupRect);
	gmPopup.m_drawContext = drawContext;
//	gmPopup.m_drawContext.m_font = smallFont;
//	gmPopup.m_drawContext.SetForeColour(255, 255, 255, 0);
	gmPopup.m_drawContext.SetBackColour(0, 0, 0, 255);

	int y1 = y0 + POPUPHEIGHT - (MARGIN * 4) - (CH * 4);
	int x1 = x0 + MARGIN;
	CGUISlider *sliderDL = gmPopup.AddSlider(x1, y1, CW, CH,  "DelaySlider");
	sliderDL->m_value = m_adsr[0];
	sliderDL->m_minVal = 0.0f;
	sliderDL->m_maxVal = 4000.0f;
	CGUISlider *sliderA = gmPopup.AddSlider(x1 + (MARGIN + CW), y1, CW, CH,  "AttackSlider");
	sliderA->m_value = m_adsr[1];
	sliderA->m_minVal = 0.0f;
	sliderA->m_maxVal = 4000.0f;
	CGUISlider *sliderD = gmPopup.AddSlider(x1 + 2*(MARGIN + CW), y1, CW, CH, "DecaySlider");
	sliderD->m_value = m_adsr[3];
	sliderD->m_minVal = 0.0f;
	sliderD->m_maxVal = 4000.0f;
	CGUISlider *sliderR = gmPopup.AddSlider(x1 + 3*(MARGIN + CW), y1, CW, CH, "ReleaseSlider");
	sliderR->m_value = m_adsr[5];
	sliderR->m_minVal = 0.0f;
	sliderR->m_maxVal = 4000.0f;
	y1 = y0 + POPUPHEIGHT - (MARGIN * 2) - (CH * 2);
	CGUISlider *sliderP = gmPopup.AddSlider(x1, y1, CW, CH, "PeakSlider");
	sliderP->m_value = m_adsr[2];
	sliderP->m_minVal = 0.0f;
	sliderP->m_maxVal = 1.0f;
	CGUISlider *sliderS = gmPopup.AddSlider(x1 + MARGIN + CW, y1, CW, CH, "SustainSlider");
	sliderS->m_value = m_adsr[4];
	sliderS->m_minVal = 0.0f;
	sliderS->m_maxVal = 1.0f;
	
	y1 = y0 + POPUPHEIGHT - (MARGIN * 3) - (CH * 3);
	CGUILabel *labelDL = gmPopup.AddLabel(x1, y1, CW, CH, "LabelDL", "D = ");
	CGUILabel *labelA = gmPopup.AddLabel(x1 + (MARGIN + CW), y1, CW, CH, "LabelA", "A = ");
	CGUILabel *labelD = gmPopup.AddLabel(x1 + 2*(MARGIN + CW), y1, CW, CH, "LabelD", "D = ");
	CGUILabel *labelR = gmPopup.AddLabel(x1 + 3*(MARGIN + CW), y1, CW, CH, "LabelR", "R = ");
	y1 = y0 + POPUPHEIGHT - (MARGIN * 1) - (CH * 1);
	CGUILabel *labelP = gmPopup.AddLabel(x1, y1, CW, CH, "LabelP", "P = ");
	CGUILabel *labelS = gmPopup.AddLabel(x0 + (MARGIN + CW), y1, CW, CH, "LabelS", "S = ");

	// Create Close button
	CGUIButton *closeButton = gmPopup.AddButton(x0 + POPUPWIDTH - MARGIN - CW, y1, CW, CH, "closeButton", "Close");

	SDL_Event event;
	bool done = false;
	while(!done)
		{
		while (SDL_PollEvent(&event))
			{
			switch (event.type)
				{
				case SDL_KEYDOWN :
					// Check for ESC pressed
					if (SDLK_ESCAPE == event.key.keysym.sym)
						done = true;
					break;
				case SDL_MOUSEMOTION :
					gmPopup.OnMouseMove(event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONDOWN :
					gmPopup.OnMouseDown(event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONUP :
					{
					CGUIControl *control = gmPopup.OnMouseUp(event.button.x, event.button.y);
					// If close button clicked, then close
					if (control == closeButton)
						done = true;
					}
					break;
				}	// end switch
			}	// wend event

		// Update labels
		char s[20];
		sprintf(s, "D=%.0fms", sliderDL->m_value);
		labelDL->SetText(s);
		sprintf(s, "A=%.0fms", sliderA->m_value);
		labelA->SetText(s);
		sprintf(s, "P=%.0f%%", sliderP->m_value * 100.0f);
		labelP->SetText(s);
		sprintf(s, "D=%.0fms", sliderD->m_value);
		labelD->SetText(s);
		sprintf(s, "S=%.0f%%", sliderS->m_value * 100.0f);
		labelS->SetText(s);
		sprintf(s, "R=%.0fms", sliderR->m_value);
		labelR->SetText(s);

		// Draw background, frame and controls
		gmPopup.DrawAllControls();

		// Draw graph "window"
		SDL_Rect r2 = popupRect;
		r2.x += MARGIN;
		r2.y += MARGIN;
		r2.h -= (MARGIN * 6 + CH * 4);
		r2.w -= (MARGIN * 2);
		SDL_RenderDrawRect(renderer, &r2);
		int x0 = r2.x + (int)(sliderDL->m_value / 100);
		int x1 = x0 + (int)(sliderA->m_value / 100);
		int x2 = x1 + (int)(sliderD->m_value / 100);
		int x3 = x2 + 40;
		int x4 = x3 + (int)(sliderR->m_value / 100);
		int h = r2.h;
		int y2 = r2.y + h;
		int yp = y2 - (int)(sliderP->m_value * h);
		int ys = y2 - (int)(sliderS->m_value * h);
		SDL_RenderDrawLine(renderer, x0, y2, x1, yp);
		SDL_RenderDrawLine(renderer, x1, yp, x2, ys);
		SDL_RenderDrawLine(renderer, x2, ys, x3, ys);
		SDL_RenderDrawLine(renderer, x3, ys, x4, y2);

		//SDL_Flip(screen);
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
		}	// wend

	// Apply the changed envelope
	m_adsr[0] = sliderDL->m_value;
	m_adsr[1] = sliderA->m_value;
	m_adsr[2] = sliderP->m_value;
	m_adsr[3] = sliderD->m_value;
	m_adsr[4] = sliderS->m_value;
	m_adsr[5] = sliderR->m_value;

	return 0;
}

int CGUIEnvelope::OnClick(const CGUIDrawContext &drawContext, int x, int y)
{
	int retval = EditADSR(drawContext);
	return retval;
}

int CGUIEnvelope::OnSwipe(int x, int y, int dx, int dy)
{
	// TODO
	return 0;
}

void CGUIEnvelope::Draw(const CGUIDrawContext &drawContext)
{
	DrawBGFill(drawContext);
	DrawFrame(drawContext);
	//drawContext.SetDrawColour(drawContext.m_foreColour); 
	//SDL_RenderDrawLine(drawContext.m_renderer, &m_rect);
	
	// calculate horizontal (time-base) scaling, while showing
	// sustain period as same as delay + attack  + decay + release
	float total = 2.0f * (m_adsr[0] + m_adsr[1] + m_adsr[3] + m_adsr[5]);
	int x0 = m_rect.x;
	int x1 = x0 + (int)(m_adsr[0] * m_rect.w / total);
	int x2 = x1 + (int)(m_adsr[1] * m_rect.w / total);
	int x3 = x2 + (int)(m_adsr[3] * m_rect.w / total);
	int x4 = x3 + m_rect.w / 2;
	int x5 = x0 + m_rect.w - 1;
	int y0 = m_rect.y + m_rect.h - 1;
	int h = m_rect.h;
	int yp = y0 - (int)(m_adsr[2] * h);
	int ys = y0 - (int)(m_adsr[4] * h);
	SDL_Colour colour = { 255, 255, 255, 0 };
	drawContext.SetDrawColour(colour); 
	SDL_RenderDrawLine(drawContext.m_renderer, x1, y0, x2, yp);
	SDL_RenderDrawLine(drawContext.m_renderer, x2, yp, x3, ys);
	SDL_RenderDrawLine(drawContext.m_renderer, x3, ys, x4, ys);
	SDL_RenderDrawLine(drawContext.m_renderer, x4, ys, x5, y0);
}

/// Set the envelope ADSR values
void CGUIEnvelope::SetADSR(float delay, float attack, float peak, float decay, float sustain, float release)
{
	m_adsr[0] = delay;
	m_adsr[1] = attack;
	m_adsr[2] = peak;
	m_adsr[3] = decay;
	m_adsr[4] = sustain;
	m_adsr[5] = release;
}

/// Get the envelope ADSR values
void CGUIEnvelope::GetADSR(float &delay, float &attack, float &peak, float &decay, float &sustain, float &release)
{
	delay = m_adsr[0];
	attack = m_adsr[1];
	peak = m_adsr[2];
	decay = m_adsr[3];
	sustain = m_adsr[4];
	release = m_adsr[5];
}



/// CGUIOptionList - Option list control

CGUIOptionList::CGUIOptionList(int x, int y, int width, int height, const char *name, const char *text)
	:	CGUIControl(x, y, width, height, CT_OPTIONLIST, name, text),
		m_selectedIndex(-1)
{

}

int CGUIOptionList::OnClick(const CGUIDrawContext &drawContext, int x, int y)
{
	if (m_selectedIndex > -1 && m_optionArray.size() > 0)
		{
		m_selectedIndex = (m_selectedIndex + 1) % m_optionArray.size();
		}

	return 0;
}

int CGUIOptionList::OnSwipe(int x, int y, int dx, int dy)
{
	if (m_selectedIndex > -1 && m_optionArray.size() > 0)
		{
		if (dx > 2)
			m_selectedIndex = (m_selectedIndex + 1) % m_optionArray.size();
		else if (dx < 2)
			{
			m_selectedIndex -= 1;
			if (-1 == m_selectedIndex)
				m_selectedIndex = (short)(m_optionArray.size() - 1);
			}
		}
		
	return 0;
}

void CGUIOptionList::Draw(const CGUIDrawContext &drawContext)
{
	DrawBGFill(drawContext);
	if (m_selectedIndex > -1 && m_selectedIndex < (short)m_optionArray.size())
		{
		DrawText(drawContext, 2, 0, m_optionArray[m_selectedIndex].c_str());
		}
	DrawFrame(drawContext);
}

void CGUIOptionList::AddOption(const char *option)
{
	std::string s(option);
	m_optionArray.push_back(s);
}

short CGUIOptionList::GetSelectedIndex() const
{
	return m_selectedIndex;
}

void CGUIOptionList::SetSelectedIndex(short index)
{
	m_selectedIndex = index;
	if (m_selectedIndex >= (short)m_optionArray.size())
		m_selectedIndex = -1;
}



///////////////////////////////////////////////////////////////////////
/// Control Manager
///////////////////////////////////////////////////////////////////////
CGUIManager::CGUIManager(SDL_Rect rect)
	:	m_drawContext(NULL, NULL),
		m_rect(rect)
{
	m_dragStartX = 0;
	m_dragStartY = 0;
	m_dragLastX = 0;
	m_dragLastY = 0;
	m_controlChanged = false;
}

/// Add a new control to the "scene"
CGUIControl *CGUIManager::AddControl(int x, int y, int w, int h, CONTROLTYPE type, const char *name, const char *text)
{
	CGUIControl *newControl = 0;
	switch (type)
		{
		case CT_LABEL :
			newControl = new CGUILabel(x, y, w, h, name, text);
			break;
		case CT_EDIT :
			newControl = new CGUIEdit(x, y, w, h, name, text);
			break;
		case CT_BUTTON :
			newControl = new CGUIButton(x, y, w, h, name, text);
			break;
		case CT_SLIDER :
			newControl = new CGUISlider(x, y, w, h, name, 0.0f, 1.0f);
			break;
		case CT_ENVELOPE :
			newControl = new CGUIEnvelope(x, y, w, h, name, text);
			break;
		case CT_OPTIONLIST :
			newControl = new CGUIOptionList(x, y, w, h, name, text);
			break;
		}

	if (newControl)
		m_controls.push_back(newControl);

	return newControl;
}

/// Shortcut - add label control
CGUILabel *CGUIManager::AddLabel(int x, int y, int w, int h, const char *name, const char *text)
{
	CGUILabel *newControl = new CGUILabel(x, y, w, h, name, text);
	if (newControl)
		m_controls.push_back(newControl);
		
	return newControl;
}

/// Shortcut - add button control
CGUIButton *CGUIManager::AddButton(int x, int y, int w, int h, const char *name, const char *text)
{
	CGUIButton *newControl = new CGUIButton(x, y, w, h, name, text);
	if (newControl)
		m_controls.push_back(newControl);
		
	return newControl;
}


/// Shortcut - add slider control
CGUISlider *CGUIManager::AddSlider(int x, int y, int w, int h, const char *name)
{
	CGUISlider *newControl = new CGUISlider(x, y, w, h, name, 0.0f, 1.0f);
	if (newControl)
		m_controls.push_back(newControl);
		
	return newControl;
}

/// Get a control by name
CGUIControl *CGUIManager::GetControl(const char *name) const
{
	for (unsigned int i = 0; i < m_controls.size(); i++)
		{
		if (0 == strcmp(name, m_controls[i]->m_name))
			return m_controls[i];
		}

	return 0;
}

/// Get a control by pixel coord
CGUIControl *CGUIManager::GetControl(int x, int y) const
{
	for (unsigned int i = 0; i < m_controls.size(); i++)
		{
		CGUIControl *control = m_controls[i];
		SDL_Rect rect = control->GetRect();
		if (x >= rect.x && x < rect.x + rect.w)
			{
			if (y >= rect.y && y < rect.y + rect.h)
				{
				return control;
				}
			}
		}

	return 0;
}

void CGUIManager::ArrangeControls(int width, int height)
{
	return;
}

void CGUIManager::DrawControl(const char *name)
{
	CGUIControl *control = GetControl(name);
	if (control)
		control->Draw(m_drawContext);
}

void CGUIManager::DrawAllControls()
{
	// draw background and frame
	m_drawContext.SetDrawColour(m_drawContext.m_backColour); 
	SDL_RenderFillRect(m_drawContext.m_renderer, &m_rect);
	m_drawContext.SetDrawColour(m_drawContext.m_foreColour); 
	SDL_RenderDrawRect(m_drawContext.m_renderer, &m_rect);

	for (unsigned int i = 0; i < m_controls.size(); i++)
		{
		CGUIControl *control = m_controls[i];
		if (control)
			control->Draw(m_drawContext);
		}

}

/// Handle mouse clicking and dragging
void CGUIManager::OnMouseDown(int x, int y)
{
	m_dragStartX = x;
	m_dragStartY = y;
	m_dragLastX = x;
	m_dragLastY = y;
}

CGUIControl *CGUIManager::OnMouseMove(int x, int y)
{
	CGUIControl *control = NULL;
	// Dragging?
	if (m_dragLastX > 0)
		{
		if (x != m_dragLastX || y != m_dragLastY)
			{
			control = ProcessSwipe(x, y, x - m_dragLastX, y - m_dragLastY);
			m_dragLastX = x;
			m_dragLastY = y;
			}
		}
		
	return control;
}

CGUIControl *CGUIManager::OnMouseUp(int x, int y)
{
	// Treat as click if mouse not moved much
	CGUIControl *control = NULL;
	if (abs(m_dragLastX - m_dragStartX) < 3 && abs(m_dragLastY - m_dragStartY) < 3)
		{
		control = ProcessClick(x, y);	
		}

	// reset drag stuff
	m_dragStartX = 0;
	m_dragStartY = 0;
	m_dragLastX = 0;
	m_dragLastY = 0;
	
	return control;
}

// Handle a mouse / touch click
CGUIControl *CGUIManager::ProcessClick(int x, int y)
{
	CGUIControl *control = GetControl(x, y);
	if (control)
		{
		// Convert coord to the controls OCS (object coordinate system)
		SDL_Rect controlRect = control->GetRect();
		int x1 = x - controlRect.x;
		int y1 = y - controlRect.y;
		control->OnClick(m_drawContext, x1, y1);
		// show updated control
		control->Draw(m_drawContext);
		m_controlChanged = true;
		return control;
		}
		
	return NULL;
}

// Process a mouse drag / swipe
CGUIControl *CGUIManager::ProcessSwipe(int x, int y, int dx, int dy)
{
	CGUIControl *control = GetControl(x, y);
	if (control)
		{
		// Convert coord to the controls OCS (object coordinate system)
		SDL_Rect controlRect = control->GetRect();
		int x1 = x - controlRect.x;
		int y1 = y - controlRect.y;
		control->OnSwipe(x1, y1, dx, dy);
		// show updated control
		control->Draw(m_drawContext);
		m_controlChanged = true;
		return control;
		}
		
	return NULL;
}



