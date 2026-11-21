#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <cstdlib>

HDC hdc; HGLRC hglrc; HWND hwnd;
float t = 0;
float bgR=0.05f, bgG=0.35f, bgB=0.75f;
int bgMode=0;
bool paused=false;
float speedMul=1.0f;
int totalFish=6;
bool nightMode=false;
bool rainMode=false;
float rainY[60], rainX[60];
int rainSoundTimer=0;

// === BUBBLE - 100 bubbles, old + new! ===
struct Bubble { float x,y, size, speed; bool active; };
Bubble bubbles[100];
int bubbleCount=20;

struct Fish { float x,y,dir,speed; float r,g,b; float fr,fg,fb; float boost; };
Fish fishes[10] = {
 {-0.6f,0.35f,1,0.0025f, 1.0f,0.85f,0.20f, 1.0f,0.60f,0.10f, 0},
 {0.5f,-0.05f,-1,0.0022f, 0.55f,0.75f,1.0f, 0.30f,0.55f,1.0f, 0},
 {-0.4f,-0.35f,1,0.0028f, 0.65f,0.45f,1.0f, 0.85f,0.30f,0.85f, 0},
 {0.3f,0.55f,1,0.0020f, 0.30f,0.90f,0.60f, 0.10f,0.70f,0.30f, 0},
 {0.0f,0.05f,-1,0.0018f, 1.0f,0.55f,0.45f, 1.0f,0.35f,0.30f, 0},
 {-0.7f,-0.15f,1,0.0023f, 0.30f,0.90f,0.95f, 0.10f,0.65f,0.85f, 0},
 {0.0f,0.60f,1,0.0020f, 1.0f,0.40f,0.60f, 1.0f,0.20f,0.40f, 0},
 {0.0f,-0.50f,-1,0.0020f, 1.0f,0.90f,0.40f, 1.0f,0.70f,0.10f, 0},
 {0.0f,0.20f,1,0.0020f, 0.50f,1.0f,0.50f, 0.20f,0.80f,0.20f, 0},
 {0.0f,-0.20f,-1,0.0020f, 0.80f,0.60f,1.0f, 0.60f,0.30f,1.0f, 0}
};

void addRandomBubbles(int n){
  for(int k=0;k<n;k++){
    if(bubbleCount>=100) return;
    // Random alli alli! - Ella kade!
    bubbles[bubbleCount].x = -0.95f + (rand()%190)/100.0f;
    bubbles[bubbleCount].y = -0.85f + (rand()%170)/100.0f;
    bubbles[bubbleCount].size = 0.012f + (rand()%25)/1000.0f;
    bubbles[bubbleCount].speed = 0.002f + (rand()%4)/1000.0f;
    bubbles[bubbleCount].active = true;
    bubbleCount++;
  }
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l){
 if(m==WM_DESTROY){ PostQuitMessage(0); return 0; }
 if(m==WM_LBUTTONDOWN){
   int mx = LOWORD(l), my = HIWORD(l); RECT rc; GetClientRect(h,&rc);
   float clickX = (mx/(float)rc.right)*2-1;
   float clickY = 1 - (my/(float)rc.bottom)*2;
   bool hitFish = false;
   // 1. Fish mele click aaytha check madi!
   for(int i=0;i<totalFish;i++){
     float dx = clickX - fishes[i].x;
     float dy = clickY - fishes[i].y;
     if(dx*dx + dy*dy < 0.09f){ // Fish body hit!
       hitFish = true;
       fishes[i].boost = 120; // 120 frames FAST!
       fishes[i].dir = (fishes[i].x < clickX ? -1 : 1); // Click inda opposite direction odutthe!
       Beep(1500,60); // Fish fast sound - chikka tik!
       Beep(1800,60);
       break;
     }
   }
   // 2. Fish illa andre - WATER click! - Sound + Bubble akkade!
   if(!hitFish){
     Beep(300,120); // Water splash - low sound!
     Beep(450,80);
     // Click madida jaga bubble!
     if(bubbleCount<100){
       bubbles[bubbleCount].x = clickX;
       bubbles[bubbleCount].y = clickY;
       bubbles[bubbleCount].size = 0.02f;
       bubbles[bubbleCount].speed = 0.004f;
       bubbles[bubbleCount].active = true;
       bubbleCount++;
     }
   }
   return 0;
 }
 if(m==WM_KEYDOWN){
   if(w==VK_LEFT){ fishes[0].x-=0.08f; fishes[0].dir=-1; }
   if(w==VK_RIGHT){ fishes[0].x+=0.08f; fishes[0].dir=1; }
   if(w==VK_UP){ fishes[0].y+=0.08f; }
   if(w==VK_DOWN){ fishes[0].y-=0.08f; }
   if(w=='B' || w=='b'){
     // OLD bubble irutthe + NEW bubble barutthe! - Delete illa!
     addRandomBubbles(20);
     // Bubble sound - blup blup!
     Beep(800,60); Beep(1000,60); Beep(1200,80);
   }
   if(w=='F' || w=='f'){ if(totalFish<10) totalFish++; else totalFish=6; Beep(700,100); }
   if(w=='C' || w=='c'){ bgMode=(bgMode+1)%4; if(bgMode==0){bgR=0.05f;bgG=0.35f;bgB=0.75f;} if(bgMode==1){bgR=0.05f;bgG=0.55f;bgB=0.60f;} if(bgMode==2){bgR=0.35f;bgG=0.20f;bgB=0.65f;} if(bgMode==3){bgR=0.70f;bgG=0.20f;bgB=0.45f;} }
   if(w=='N' || w=='n'){ nightMode=!nightMode; Beep(500,200); }
   if(w=='R' || w=='r'){ rainMode=!rainMode; Beep(400,200); if(rainMode){ Beep(250,300); Beep(300,300); } }
   if(w=='P' || w=='p'){ paused=!paused; }
   if(w==VK_ADD || w==VK_OEM_PLUS){ speedMul+=0.3f; }
   if(w==VK_SUBTRACT || w==VK_OEM_MINUS){ speedMul-=0.3f; if(speedMul<0.2f) speedMul=0.2f; }
   return 0;
 }
 return DefWindowProc(h,m,w,l);
}

void circle(float cx,float cy,float rx,float ry, float r,float g,float b){
 glColor3f(r,g,b);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
 for(int i=0;i<=30;i++){ float a=i*3.14f*2/30; glVertex2f(cx+cos(a)*rx, cy+sin(a)*ry); }
 glEnd();
}
void sparkle(float cx, float cy, float s){
 glColor3f(1,1,1);
 glBegin(GL_TRIANGLES);
 glVertex2f(cx, cy+s); glVertex2f(cx+s*0.25f, cy); glVertex2f(cx-s*0.25f, cy);
 glVertex2f(cx, cy-s); glVertex2f(cx+s*0.25f, cy); glVertex2f(cx-s*0.25f, cy);
 glVertex2f(cx+s, cy); glVertex2f(cx, cy+s*0.25f); glVertex2f(cx, cy-s*0.25f);
 glVertex2f(cx-s, cy); glVertex2f(cx, cy+s*0.25f); glVertex2f(cx, cy-s*0.25f);
 glEnd();
}
void drawBeautifulFish(float x, float y, float dir, float s, float R,float G,float B, float FR,float FG,float FB, int idx){
 glPushMatrix(); glTranslatef(x,y,0); glScalef(s*dir,s,1);
 // FAST hogthidre - Speed lines!
 if(fishes[idx].boost>0){
   glColor3f(1,1,1);
   glBegin(GL_LINES);
   glVertex2f(-0.45f,0.10f); glVertex2f(-0.65f,0.10f);
   glVertex2f(-0.45f,0.0f); glVertex2f(-0.70f,0.0f);
   glVertex2f(-0.45f,-0.10f); glVertex2f(-0.65f,-0.10f);
   glEnd();
 }
 glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColor4f(R,G,B,0.18f);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(0,0);
 for(int i=0;i<=30;i++){ float a=i*3.14f*2/30; glVertex2f(cos(a)*0.38f, sin(a)*0.28f); }
 glEnd(); glDisable(GL_BLEND);
 glColor3f(FR,FG,FB);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(-0.20f,0);
 for(int i=0;i<=20;i++){ float a=-1.3f+i*0.13f; glVertex2f(-0.44f+cos(a)*0.02f, sin(a)*0.21f); }
 glEnd();
 glColor3f(FR*0.7f+0.3f, FG*0.7f+0.3f, FB*0.7f+0.3f);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(-0.20f,0);
 for(int i=0;i<=20;i++){ float a=-1.0f+i*0.10f; glVertex2f(-0.36f+cos(a)*0.02f, sin(a)*0.14f); }
 glEnd();
 for(int layer=0; layer<12; layer++){
   float tt = layer/12.0f; float shade = 0.75f + tt*0.35f;
   glColor3f(R*shade, G*shade, B*shade);
   float y0 = -0.20f + tt*0.40f; float y1 = -0.20f + (tt+0.12f)*0.40f;
   glBegin(GL_QUAD_STRIP);
   for(int i=0;i<=20;i++){ float ang = i*3.14f/20.0f; float bx = -cos(ang)*0.28f; float hh = 0.20f * sqrt(1 - (bx/0.28f)*(bx/0.28f)*0.85f); glVertex2f(bx, y0*(hh/0.20f)); glVertex2f(bx, y1*(hh/0.20f)); }
   glEnd();
 }
 glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColor4f(1,1,1,0.35f);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(-0.10f,0.10f);
 for(int i=0;i<=15;i++){ float a=i*3.14f*2/15; glVertex2f(-0.10f+cos(a)*0.09f, 0.10f+sin(a)*0.04f); }
 glEnd(); glDisable(GL_BLEND);
 glColor3f(FR,FG,FB);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(-0.10f,0.14f); glVertex2f(0.10f,0.14f); glVertex2f(0.05f,0.26f); glVertex2f(-0.05f,0.26f); glEnd();
 glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColor4f(FR,FG,FB,0.75f);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(0.02f,-0.02f);
 for(int i=0;i<=14;i++){ float a=-1.2f+i*0.18f; glVertex2f(0.02f+cos(a)*0.12f, -0.02f+sin(a)*0.06f); }
 glEnd(); glDisable(GL_BLEND);
 float sp = sin(t*3 + idx*2)*0.5f+0.5f;
 if(sp>0.3f && !nightMode){ sparkle(-0.10f, 0.02f, 0.025f); sparkle(-0.02f, -0.08f, 0.018f); }
 circle(0.18f,0.06f,0.058f,0.058f,1,1,1);
 circle(0.19f,0.06f,0.040f,0.040f,0.10f,0.30f,0.90f);
 circle(0.19f,0.06f,0.032f,0.032f,0.25f,0.55f,1.0f);
 circle(0.195f,0.06f,0.020f,0.020f,0.05f,0.05f,0.15f);
 circle(0.207f,0.080f,0.010f,0.010f,1,1,1);
 if(nightMode){ glColor3f(0.05f,0.05f,0.15f); glBegin(GL_LINES); glVertex2f(0.15f,0.06f); glVertex2f(0.22f,0.06f); glEnd(); }
 glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColor4f(1,0.55f,0.70f,0.65f);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(0.14f,-0.06f);
 for(int i=0;i<=15;i++){ float a=i*3.14f*2/15; glVertex2f(0.14f+cos(a)*0.035f, -0.06f+sin(a)*0.022f); }
 glEnd(); glDisable(GL_BLEND);
 glColor3f(0.75f,0.10f,0.25f);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(0.285f,-0.01f);
 for(int i=0;i<=16;i++){ float a=i*3.14f*2/16; glVertex2f(0.285f+cos(a)*0.028f, -0.01f+sin(a)*0.020f); }
 glEnd();
 glColor3f(1,0.60f,0.75f);
 glBegin(GL_TRIANGLE_FAN); glVertex2f(0.285f,-0.012f);
 for(int i=0;i<=12;i++){ float a=i*3.14f*2/12; glVertex2f(0.285f+cos(a)*0.015f, -0.012f+sin(a)*0.010f); }
 glEnd();
 if(idx==0){ glColor3f(1,0.85f,0.0f); glBegin(GL_TRIANGLES); glVertex2f(-0.05f,0.26f); glVertex2f(0.05f,0.26f); glVertex2f(0,0.34f); glEnd(); circle(0,0.30f,0.010f,0.010f,1,0,0); }
 glPopMatrix();
}
void drawPlant(float bx, float h, float r,float g,float b){
 for(int j=0;j<4;j++){ float sway = sin(t + bx*3 + j)*0.05f; float lx = bx + j*0.04f; glColor3f(r,g,b); glBegin(GL_TRIANGLE_FAN); glVertex2f(lx, -0.88f); for(int a=0;a<=14;a++){ float ang=a*3.14f*2/14; glVertex2f(lx+cos(ang)*0.028f+sway, -0.88f + h*0.5f + sin(ang)*h*0.5f); } glEnd(); }
 for(int k=0;k<5;k++){ float ly = -0.88f + h*k/5.0f; float lx = bx + sin(k*1.5f)*0.03f + sin(t+bx)*0.03f; circle(lx,ly,0.025f,0.06f,r*0.9f,g*0.9f,b*0.9f); }
}
void render(){
 float curR=bgR, curG=bgG, curB=bgB;
 if(nightMode){ curR=0.02f; curG=0.05f; curB=0.20f; }
 if(rainMode && ((int)(t*10)%50==0)){ curR=0.85f; curG=0.85f; curB=0.95f; }
 glClearColor(curR,curG,curB,1); glClear(GL_COLOR_BUFFER_BIT);
 for(int i=0;i<25;i++){ float tt=i/25.0f; glColor3f(curR+tt*0.10f, curG+tt*0.25f, curB+tt*0.05f); glBegin(GL_QUADS); glVertex2f(-1,-1+tt*2); glVertex2f(1,-1+tt*2); glVertex2f(1,-1+(tt+0.05f)*2); glVertex2f(-1,-1+(tt+0.05f)*2); glEnd(); }
 if(nightMode){ circle(0.65f,0.75f,0.12f,0.12f,1,0.95f,0.75f); circle(0.60f,0.78f,0.03f,0.03f,0.85f,0.80f,0.60f); for(int s=0;s<20;s++){ circle(-0.9f+(s%10)*0.18f, 0.60f+(s/10)*0.25f, 0.008f,0.008f,1,1,1); } }
 glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glColor4f(1,1,1,0.12f); glBegin(GL_TRIANGLES); glVertex2f(-0.2f,1); glVertex2f(0.2f,1); glVertex2f(0,-0.3f); glEnd(); glDisable(GL_BLEND);
 glColor3f(0.05f,0.05f,0.08f); glBegin(GL_QUADS); glVertex2f(-1,-1); glVertex2f(1,-1); glVertex2f(1,-0.75f); glVertex2f(-1,-0.75f); glEnd();
 for(int i=0;i<40;i++){ float gx=-1+(i%20)*0.1f; float gy=-0.98f+(i/20)*0.08f; if(i%3==0) circle(gx,gy,0.028f,0.016f,0.25f,0.25f,0.30f); else if(i%3==1) circle(gx,gy,0.025f,0.014f,0.35f,0.30f,0.35f); else circle(gx,gy,0.022f,0.013f,0.18f,0.20f,0.25f); }
 drawPlant(-0.88f,0.55f, 0.15f,0.75f,0.30f); drawPlant(-0.65f,0.85f, 0.10f,0.65f,0.25f); drawPlant(-0.30f,0.45f, 0.20f,0.80f,0.35f); drawPlant(0.45f,0.75f, 0.12f,0.70f,0.28f); drawPlant(0.70f,0.95f, 0.10f,0.60f,0.35f); drawPlant(0.88f,0.50f, 0.18f,0.78f,0.32f); drawPlant(0.05f,0.35f, 0.25f,0.85f,0.40f);
 circle(-0.45f,-0.80f,0.09f,0.12f,1,0.55f,0.70f); circle(-0.38f,-0.82f,0.07f,0.09f,1,0.70f,0.80f); circle(0.55f,-0.82f,0.08f,0.10f,1,0.65f,0.35f);

 // RAIN + RAIN SOUND!
 if(rainMode){
   glColor3f(0.70f,0.85f,1.0f); glBegin(GL_LINES); for(int i=0;i<60;i++){ glVertex2f(rainX[i], rainY[i]); glVertex2f(rainX[i]-0.02f, rainY[i]+0.08f); } glEnd();
   if(!paused){
     for(int i=0;i<60;i++){ rainY[i]-=0.025f; if(rainY[i]<-1){ rainY[i]=1; rainX[i]=-1+(rand()%200)/100.0f; } }
     rainSoundTimer++;
     if(rainSoundTimer>40){ rainSoundTimer=0; Beep(200+rand()%150, 60); } // shhh shhh - male sound!
   }
 }

 // BUBBLES - Random alli alli, old + new!
 glColor3f(1,1,1);
 for(int i=0;i<bubbleCount;i++){
   if(!bubbles[i].active) continue;
   glBegin(GL_LINE_LOOP); for(int a=0;a<12;a++){ float ang=a*3.14f/6; glVertex2f(bubbles[i].x+cos(ang)*bubbles[i].size, bubbles[i].y+sin(ang)*bubbles[i].size);} glEnd();
   glBegin(GL_POINTS); glPointSize(2); glVertex2f(bubbles[i].x+0.01f,bubbles[i].y+0.01f); glEnd();
   if(!paused){
     bubbles[i].y += bubbles[i].speed;
     bubbles[i].x += sin(t*2+i)*0.0005f;
     if(bubbles[i].y>1.0f){
       // Mele hogidre - Matt e kelage random jaga!
       bubbles[i].y = -0.85f + (rand()%170)/100.0f;
       bubbles[i].x = -0.95f + (rand()%190)/100.0f;
     }
   }
 }
 // Auto random bubble - 5 sec ge 1 hosa bubble!
 if(!paused && ((int)(t*20)%200==0)){ addRandomBubbles(1); }

 for(int i=0;i<totalFish;i++){
   drawBeautifulFish(fishes[i].x, fishes[i].y, fishes[i].dir, 0.62f, fishes[i].r, fishes[i].g, fishes[i].b, fishes[i].fr, fishes[i].fg, fishes[i].fb, i);
   if(!paused){
     if(i!=0){
       float curSpeed = fishes[i].speed * speedMul;
       if(fishes[i].boost>0){ curSpeed *= 5.0f; fishes[i].boost--; } // FAST!
       fishes[i].x += curSpeed*fishes[i].dir;
       fishes[i].y += sin(t+i*2)*0.0007f;
       if(fishes[i].x>0.82f) fishes[i].dir=-1;
       if(fishes[i].x<-0.82f) fishes[i].dir=1;
     } else {
       if(fishes[i].boost>0) fishes[i].boost--;
     }
   }
 }
 if(!paused) t+=0.05f;
 SwapBuffers(hdc);
}
int WINAPI WinMain(HINSTANCE hi,HINSTANCE p,LPSTR c,int n){
 for(int i=0;i<20;i++){ bubbles[i].x=-0.95f+(rand()%190)/100.0f; bubbles[i].y=-0.85f+(rand()%170)/100.0f; bubbles[i].size=0.012f+(rand()%25)/1000.0f; bubbles[i].speed=0.002f+(rand()%4)/1000.0f; bubbles[i].active=true; }
 for(int i=20;i<100;i++) bubbles[i].active=false;
 for(int i=0;i<60;i++){ rainY[i]=-1+(rand()%200)/100.0f; rainX[i]=-1+(rand()%200)/100.0f; }
 WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=hi; wc.lpszClassName="Fish"; wc.hCursor=LoadCursor(0,IDC_ARROW);
 RegisterClass(&wc);
 hwnd=CreateWindow("Fish","Aquarium - Click Fish=Fast, Click Water=Sound, B=Bubble, R=Rain Sound",WS_OVERLAPPEDWINDOW,100,100,900,600,0,0,hi,0);
 hdc=GetDC(hwnd); PIXELFORMATDESCRIPTOR pfd={sizeof(pfd),1,PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,PFD_MAIN_PLANE,0,0,0,0};
 int pf=ChoosePixelFormat(hdc,&pfd); SetPixelFormat(hdc,pf,&pfd); hglrc=wglCreateContext(hdc); wglMakeCurrent(hdc,hglrc);
 ShowWindow(hwnd,n);
 MSG msg; while(true){ if(PeekMessage(&msg,0,0,0,PM_REMOVE)){ if(msg.message==WM_QUIT) break; TranslateMessage(&msg); DispatchMessage(&msg);} render(); Sleep(16); }
 return 0;
}