#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include "imageloader.h"
#include <iostream>
#include <time.h>

using namespace std;

static int slices = 16;
static int stacks = 16;

const int minX = -90;
const int maxX = 90;

const int minY = -90;
const int maxY = 90;

const float g_translation_speed = 0.1;
const float g_rotation_speed = M_PI/180*0.1;

bool keyPressedUP = false, keyPressedDOWN = false, keyPressedLEFT = false, keyPressedRIGHT = false;

float carSpeed = 0.15f;

float RandomFloat(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

int RandomInt(int min, int max) {
    return (rand() & max) + min;
}

//*********** Sterowanie kamery ****************

class camera{
    private:
        float x1, y1, z1;
        float lx1, ly1, lz1;
        float keyMove1;
        float keyMove2;
        bool ghostMode1;
        float yaw1;
        float pitch1;
        float strefeX;
        float strefeY;
    public:
        camera(): x1(0), y1(0), z1(0), lx1(0), ly1(0), lz1(0),
        keyMove1(0), keyMove2(0), ghostMode1(false), yaw1(0), pitch1(0), strefeX(0), strefeY(0){}

        camera(float x, float y, float z, float lx, float ly, float lz):x1(x), y1(y), z1(z), lx1(lx), ly1(ly), lz1(lz),
        keyMove1(0), keyMove2(0), ghostMode1(false), yaw1(0), pitch1(0), strefeX(0), strefeY(0){}

        float & x() { return x1; }
        float & y() { return y1; }
        float & z() { return z1; }
        float & lx() { return lx1; }
        float & ly() { return ly1; }
        float & lz() { return lz1; }
        float & keyMove(){ return keyMove1; }
        float & keyMoveS(){ return keyMove2; }
        float & pitch() { return pitch1; }
        float & yaw() { return yaw1; }
        bool & ghostMode() { if(ghostMode1 == false) { z() = 1.0f;  } return ghostMode1; }

        void refresh(){

            lx() = cos(yaw1) * cos(pitch1);
            ly() = sin(yaw1) * cos(pitch1);
            lz() = sin(pitch1);

            strefeX = cos(yaw1 - M_PI_2);
            strefeY = sin(yaw1 - M_PI_2);

            gluLookAt(   x(),        y(),        z(),
                      x() + lx(), y() + ly(), z() + lz(),
                         0,0,1);
        }

        void updatePositionKey(float speed){
            x() += keyMove() * lx() * speed;
            y() += keyMove() * ly() * speed;

            if(ghostMode() == true){
                if(z() >= 1 || lz()*4 >= z()) {
                    z() += keyMove() * lz() * (speed*2);
                }
                x() += keyMove() * lx() * (speed*2);
                y() += keyMove() * ly() * (speed*2);
            } else {
                x() += keyMove() * lx() * (speed);
                y() += keyMove() * ly() * (speed);
            }
        }

        void updatePositionKeyS(float speed){
            if(ghostMode() == true){
                x() = x() + (speed*2)*strefeX*keyMoveS();
                y() = y() + (speed*2)*strefeY*keyMoveS();
            } else {
                x() = x() + speed*strefeX*keyMoveS();
                y() = y() + speed*strefeY*keyMoveS();
            }
        }

        void rotateYaw(float angle){
            yaw1 += angle;
        }

        void rotatePitch(float angle){
            const float limit = 89.0 * M_PI / 180.0;
            pitch1 += angle;
            if(pitch1 < -limit) { pitch1 = -limit; }
            if(pitch1 > limit) { pitch1 = limit; }
        }

};

class snowMan{
    private:
      unsigned int n;
      float ** snowmans;
      float ** target;
    public:
        snowMan(): n(0) {
            snowmans = new float*[n];
            target = new float*[n];
            for(unsigned i = 0; i<n; i++){
                snowmans[i] = new float[5];
                target[i] = new float[2];
            }
        }
        snowMan(int number): n(number){
            snowmans = new float*[n];
            target = new float*[n];
            for(unsigned i = 0; i<n; i++){
                snowmans[i] = new float[5];
                snowmans[i][0] = RandomFloat(minX, maxX); // pozycja X
                snowmans[i][1] = RandomFloat(minY, maxY); // pozycja y
                snowmans[i][2] = 0.0; // pozycja z
                snowmans[i][3] = true; // true - rysuje, false - nie rysuje
                snowmans[i][4] = 1; // kat

                target[i] = new float[2];
                target[i][0] = RandomFloat(minX, maxX);
                target[i][1] = RandomFloat(minY, maxY);
            }
        }
        float & operator() (unsigned n, unsigned m){ return snowmans[n][m]; }
        unsigned number(){ return n; }
        void set(int n, int m, int value){ snowmans[n][m] = value; }

        float getTarget(int n, int m){ return target[n][m]; }

        void setNewTargets(){
            for(unsigned i = 0; i<n; i++){
                if(sqrt(pow(target[i][0]-snowmans[i][0],2)+pow(target[i][1]-snowmans[i][1],2)) < 10){
                    target[i][0] = RandomFloat(minX, maxX);
                    target[i][1] = RandomFloat(minY, maxY);
                }
            }
        }

        void draw(){
            for(unsigned i = 0; i<n; i++){
                if(snowmans[i][3] == true){
                    glPushMatrix();
                       glTranslatef(snowmans[i][0], snowmans[i][1], snowmans[i][2]);
                       glRotatef(snowmans[i][4], 0,0,1);
                        glColor3f(0.9, 0.9, 0.9);
                        glPushMatrix();
                            glPushMatrix();
                              glTranslatef(0.0, 0.0, 0.5);
                              glutSolidSphere(0.5, 20, 20);
                            glPopMatrix();

                            glPushMatrix();
                              glTranslatef(0.0, 0.0, 1.0);
                              glutSolidSphere(0.3, 20, 20);
                            glPopMatrix();

                            glPushMatrix();
                                glTranslatef(0.0, 0.0, 1.3);
                                glutSolidSphere(0.2, 20, 20);

                                glColor3d(0.0,0.0,0.0);
                                glPushMatrix();
                                    glTranslatef(0.08, -0.2, 0.05);
                                    glutSolidSphere(0.05, 20, 20);
                                glPopMatrix();

                                glPushMatrix();
                                    glTranslatef(-0.08, -0.2, 0.05);
                                    glutSolidSphere(0.05, 20, 20);
                                glPopMatrix();

                                glColor3d(0.8,0.2,0.2);
                                glPushMatrix();
                                    glTranslatef(0.0,-0.2,0.0);
                                    glRotatef(90, 1,0,0);
                                    glutSolidCone(0.04,0.3, slices, stacks);
                                glPopMatrix();

                                glColor3d(0.2,0.2,0.2);
                                glPushMatrix();
                                    glTranslatef(0.0,0.0,0.15);
                                    glutSolidCone(0.1,0.3, slices, stacks);
                                glPopMatrix();
                            glPopMatrix();
                        glPopMatrix();
                    glPopMatrix();
                }
            }
        }

        ~snowMan(){
            for(unsigned i = 0; i<n; i++){
                delete [] snowmans[i];
            }
            delete [] snowmans;

            for(unsigned i = 0; i<n; i++){
                delete [] target[i];
            }
            delete [] target;
        }

};

camera cam(0.0f, 5.0f, 1.0f, 0.0f, 1.0f, 0.0f);
snowMan snow(50); // 50 bałwanków

void moveSnowMans(int){
    for(unsigned i = 0; i<snow.number(); i++){
        float dX = snow.getTarget(i,0) - snow(i,0);
        float dY = snow.getTarget(i,1) - snow(i,1);
        float angle = atan2(dY,dX)*180/M_PI;

        snow(i,0) += (dX)*0.003;
        snow(i,1) +=  (dY)*0.003;
        snow(i,4) = angle;
    }
    snow.setNewTargets();
    glutTimerFunc(10, moveSnowMans, 0);
}

class bulet{
    private:
        float x1;
        float y1;
        float z1;
        float vx1;
        float vy1;
        float vz1;
        bool draw1;
    public:
        bulet(): x1(0), y1(0), z1(0), vx1(0), vy1(0), vz1(0), draw1(false) {}
        bulet(float x, float y, float z, bool draw1): x1(x), y1(y), z1(z), vx1(0), vy1(0), vz1(0), draw1(draw1) {}
        float & x() { return x1; }
        float & y() { return y1; }
        float & z() { return z1; }
        float & vx() { return vx1; }
        float & vy() { return vy1; }
        float & vz() { return vz1; }
        bool & draw() { return draw1; }
        void setVector(float x, float y, float z){ vx1 = x; vy1 = y; vz1 = z; };

        void drawBulet(){
            if(draw() == true){
                glPushMatrix();
                glColor3d(10,10,10);
                  glTranslatef(x(), y(), z());
                  glutSolidSphere(0.2, 16, 16);
                glPopMatrix();
            }
        }

        void setPosition(float x, float y, float z){
              this->x() = x;
              this->y() = y;
              this->z() = z;
        }

        bool calculateColision(){
            for(unsigned int i = 0; i < snow.number(); ++i){
                if(sqrt(pow(x()-snow(i,0),2)+pow(y()-snow(i,1),2)+pow(z()-snow(i,2)-0.5,2)) < 1){
                    snow(i,3) = false;
                    return true;
                }
            }
            return false;
        }
};

bulet bul(cam.x(), cam.y(), cam.z(), false);

void drawBulet1(int value){
    float i = 0;
    i += 3;
    bul.x() += bul.vx()*i;
    bul.y() += bul.vy()*i;
    bul.z() += bul.vz()*i;
    if(bul.calculateColision()){
        bul.draw() = false;
    }
    glutTimerFunc(1, drawBulet1, 0);
}


//********** Funkcja do wczytywania tekstur **************
GLuint loadTexture(Image* image) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
  return textureId;
}
GLuint texture[16];
GLUquadric *quad;
//********************************************************


class car{
    private:
       float x1;
       float y1;
       float z1;
       float rotate1;
       float angle1;
    public:
        car(): x1(0), y1(0), z1(1), rotate1(0), angle1(0) {}
        car(const float & x, const float & y, const float & z, const float & r, const float & angle):
          x1(x), y1(y), z1(z), rotate1(r), angle1(angle) {}

        float & x(){ return x1; }
        float & y(){ return y1; }
        float & z(){ return z1; }
        float & rotate(){ return rotate1; }
        float & angle(){ return angle1; }

        const float & x()const { return x1; }
        const float & y()const { return y1; }
        const float & z()const { return z1; }
        const float & rotate()const { return rotate1; }
        const float & angle()const { return angle1; }

        void draw(){
          glTranslatef(x1,y1,z1);
          glRotatef(angle1*60, 0, 0, 1);
            glPushMatrix();
                glPushMatrix();
                  glScaled(2.5,1,0.1);
                  glTranslated(0,0,-6);
                  glutSolidCube(2);
                glPopMatrix();

                glPushMatrix();
                  glColor3f(0,0,0);
                  glRotated(90,1,0,0);
                  glTranslated(1.5,-0.65,1.05);
                  glRotated(rotate1,0,0,1);
            //      glRotated(angle1*60,0,1,0);
                  glutSolidTorus(0.05,0.3,26,26);
                  glColor3f(0.3,0.3,0.3);
                  glutSolidSphere(0.05,16,16);

                  glLineWidth(30);
                  glBegin(GL_LINES);
                      glVertex3f(0,0,0);
                      glVertex3f(0,0.3,0);

                      glVertex3f(0,0,0);
                      glVertex3f(0,-0.3,0);

                      glVertex3f(0,0,0);
                      glVertex3f(0.3,0,0);

                      glVertex3f(0,0,0);
                      glVertex3f(-0.3,0,0);

                      glVertex3f(0,0,0);
                      glVertex3f(0.2,0.2,0);

                      glVertex3f(0,0,0);
                      glVertex3f(0.2,-0.2,0);

                      glVertex3f(0,0,0);
                      glVertex3f(-0.2,-0.2,0);

                      glVertex3f(0,0,0);
                      glVertex3f(-0.2,0.2,0);
                  glEnd();
                glPopMatrix();

                glPushMatrix();
                     glColor3f(0,0,0);
                     glRotated(90,1,0,0);

                     glTranslated(1.5,-0.65,-1.05);
                     glRotated(rotate1,0,0,1);
                   //  glRotated(angle1*60,0,1,0);
                     glutSolidTorus(0.05,0.3,26,26);

                     glColor3f(0.3,0.3,0.3);
                     glutSolidSphere(0.05,16,16);

                     glLineWidth(30);
                     glBegin(GL_LINES);
                         glVertex3f(0,0,0);
                         glVertex3f(0,0.3,0);

                         glVertex3f(0,0,0);
                         glVertex3f(0,-0.3,0);

                         glVertex3f(0,0,0);
                         glVertex3f(0.3,0,0);

                         glVertex3f(0,0,0);
                         glVertex3f(-0.3,0,0);

                         glVertex3f(0,0,0);
                         glVertex3f(0.2,0.2,0);

                         glVertex3f(0,0,0);
                         glVertex3f(0.2,-0.2,0);

                         glVertex3f(0,0,0);
                         glVertex3f(-0.2,-0.2,0);

                         glVertex3f(0,0,0);
                         glVertex3f(-0.2,0.2,0);
                     glEnd();
                glPopMatrix();

                glPushMatrix();
                    glColor3f(0,0,0);
                    glRotated(90,1,0,0);
                    glTranslated(-1.5,-0.65,1.05);
                    glutSolidTorus(0.05,0.3,26,26);
                    glRotated(rotate1,0,0,1);
                    glColor3f(0.3,0.3,0.3);
                    glutSolidSphere(0.05,16,16);

                    glLineWidth(30);
                    glBegin(GL_LINES);
                        glVertex3f(0,0,0);
                        glVertex3f(0,0.3,0);

                        glVertex3f(0,0,0);
                        glVertex3f(0,-0.3,0);

                        glVertex3f(0,0,0);
                        glVertex3f(0.3,0,0);

                        glVertex3f(0,0,0);
                        glVertex3f(-0.3,0,0);

                        glVertex3f(0,0,0);
                        glVertex3f(0.2,0.2,0);

                        glVertex3f(0,0,0);
                        glVertex3f(0.2,-0.2,0);

                        glVertex3f(0,0,0);
                        glVertex3f(-0.2,-0.2,0);

                        glVertex3f(0,0,0);
                        glVertex3f(-0.2,0.2,0);
                    glEnd();
                glPopMatrix();

                glPushMatrix();
                   glColor3f(0,0,0);
                   glRotated(90,1,0,0);
                   glTranslated(-1.5,-0.65,-1.05);
                   glutSolidTorus(0.05,0.3,26,26);
                   glRotated(rotate1,0,0,1);
                   glColor3f(0.3,0.3,0.3);
                   glutSolidSphere(0.05,16,16);

                   glLineWidth(30);
                   glBegin(GL_LINES);
                       glVertex3f(0,0,0);
                       glVertex3f(0,0.3,0);

                       glVertex3f(0,0,0);
                       glVertex3f(0,-0.3,0);

                       glVertex3f(0,0,0);
                       glVertex3f(0.3,0,0);

                       glVertex3f(0,0,0);
                       glVertex3f(-0.3,0,0);

                       glVertex3f(0,0,0);
                       glVertex3f(0.2,0.2,0);

                       glVertex3f(0,0,0);
                       glVertex3f(0.2,-0.2,0);

                       glVertex3f(0,0,0);
                       glVertex3f(-0.2,-0.2,0);

                       glVertex3f(0,0,0);
                       glVertex3f(-0.2,0.2,0);
                   glEnd();
                glPopMatrix();
            glPopMatrix();
        }
};

car samochod(0,0,1,0,0);

void drawSpace(int a){
      glColor3d(1,1,1);

      //oświetlenie planet
      GLfloat white1[] = {0.4f, 0.4f, 0.4f, 1.0f};
      GLfloat lightPos1[] = {0.0f, 0.0f, 0.0f, 1.0f};
      glLightfv(GL_LIGHT2, GL_DIFFUSE, white1);
      glLightfv(GL_LIGHT2, GL_POSITION, lightPos1);

      GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
      GLfloat lightPos2[] = {0.0f, 0.0f, 0.0f, 1.0f};
      glLightfv(GL_LIGHT3, GL_SPECULAR , white); // refleksy na planetach
      glLightfv(GL_LIGHT3, GL_POSITION, lightPos2);

      GLfloat lightColorR[] = {1.0f, 1.0f, 1.0f, 1.0f};

      GLfloat fPozycja_reflektora[4] = {0.0f, 0.0f, -100.0f, 1.0f};
      GLfloat fKierunek_reflektora[4] = {0.0f, 0.0f, 4.0f, 1.0f};

      glLightfv(GL_LIGHT4, GL_DIFFUSE, lightColorR);
      glLightfv(GL_LIGHT4, GL_POSITION, fPozycja_reflektora);
      glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, fKierunek_reflektora);
      glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 3.0f);
      glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, 0.0f);

      GLfloat fPozycja_reflektoraG[4] = {0.0f, 0.0f,  100.0f, 1.0f};
      GLfloat fKierunek_reflektoraG[4] = {0.0f, 0.0f, -4.0f, 1.0f};
      glLightfv(GL_LIGHT5, GL_DIFFUSE, lightColorR);
      glLightfv(GL_LIGHT5, GL_POSITION, fPozycja_reflektoraG);
      glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, fKierunek_reflektoraG);
      glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 3.0f);
      glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 0.0f);

      glMatrixMode(GL_MODELVIEW);
            glRotated(180,0,1,0);

            // Słońce
            glRotated(a/6,0,0,0.2);
            glBindTexture(GL_TEXTURE_2D, texture[1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            gluQuadricTexture(quad,1);
            gluSphere(quad,4,16,16);

            // Merkury
            glPushMatrix();
              glRotated(a,0,0,1);
              glTranslated(0,5.8,-1);
              glRotated(a,0,0,1);
              glBindTexture(GL_TEXTURE_2D, texture[4]);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              gluQuadricTexture(quad,1);
              gluSphere(quad,0.76,16,16);
            glPopMatrix();

            //Wenus
            glPushMatrix();
              glRotated(a/2.54,0,0,1);
              glTranslated(0,-10.8,-1);
              glRotated(a,0,0,1);
              glBindTexture(GL_TEXTURE_2D, texture[5]);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              gluQuadricTexture(quad,1);
              gluSphere(quad,1.88,16,16);
            glPopMatrix();

            //Ziemia
            glPushMatrix();
              glRotated(a/4.14,0,0,1);
              glTranslated(0,-15,-1);
              glRotated(a,0,0,1);
              glBindTexture(GL_TEXTURE_2D, texture[0]);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              gluQuadricTexture(quad,1);
              gluSphere(quad,2,16,16);

              glRotated(a/4,0,0,1);
              glTranslated(0,-3,-1);
              glRotated(a,0,0,1);
              glBindTexture(GL_TEXTURE_2D, texture[2]);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              gluQuadricTexture(quad,1);
              gluSphere(quad,0.5,16,16);
            glPopMatrix();

            //Mars
            glPushMatrix();
              glRotated(a/7.79,0,0,1);
              glTranslated(0,-22.8,-1);
              glRotated(a,0,0,1);
              glBindTexture(GL_TEXTURE_2D, texture[3]);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              gluQuadricTexture(quad,1);
              gluSphere(quad,1.06,16,16);
            glPopMatrix();

            //Jowisz
            glPushMatrix();
              glRotated(a/49,0,0,1);
              glTranslated(0,-77.5/2,-1);
              glRotated(a,0,0,1);
              glBindTexture(GL_TEXTURE_2D, texture[6]);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              gluQuadricTexture(quad,1);
              gluSphere(quad,22/4,16,16);
            glPopMatrix();

            //Saturn
            glPushMatrix();
              glRotated(a/122,0,0,1);
              glTranslated(0,-142.3/2,-1);
              glRotated(a,0,0,1);
              glBindTexture(GL_TEXTURE_2D, texture[7]);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
              gluQuadricTexture(quad,1);
              gluSphere(quad,18.88/4,16,16);
            glPopMatrix();
        glPopMatrix();
}

float pointX(float u1, float v1){
    return (-90*pow(u1,5)+225*pow(u1,4)-270*pow(u1,3)+180*pow(u1,2)-45*u1)*cos(M_PI*v1);
}

float pointY(float u1, float v1){
    return 160*pow(u1,4)-320*pow(u1,3)+160*pow(u1,2);
}

float pointZ(float u1, float v1){
    return (-90*pow(u1,5)+225*pow(u1,4)-270*pow(u1,3)+180*pow(u1,2)-45*u1)*sin(M_PI*v1);
}

void drawEgg(){
    glColor3d(1,1,1);
    glBegin(GL_QUADS);
        float x1 = 0, x2 = 0, x3 = 0, x4 = 0;
        float y1 = 0, y2 = 0, y3 = 0, y4 = 0;
        float z1 = 0, z2 = 0, z3 = 0, z4 = 0;
        float arg = 0.025;

        for(float i = 0; i<=1-arg; i+=arg){
            for(float j = 0; j<=1-arg; j+=arg){
                 x1 = pointX(i,j); y1 = pointY(i,j); z1 = pointZ(i,j);
                 x2 = pointX(i+arg,j); y2 = pointY(i+arg,j);  z2 = pointZ(i+arg,j);
                 x3 = pointX(i+arg,j+arg); y3 = pointY(i+arg,j+arg); z3 = pointZ(i+arg,j+arg);
                 x4 = pointX(i,j+arg); y4 = pointY(i,j+arg); z4 = pointZ(i,j+arg);

                 glColor3d(0,0,1);
                 glVertex3f( x1 , y1 , z1);

                 glColor3d(1,0,0);
                 glVertex3f( x2 , y2 , z2);

                 glColor3d(0,1,1);
                 glVertex3f( x3 , y3 , z3);

                 glColor3d(1,1,0);
                 glVertex3f( x4 , y4 , z4);
            }
        }
    glEnd();
}

static void resize(int w, int h) {
    float ratio =  ((float) w) / ((float) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, ratio, 1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

static void display(void) {
    const double t = glutGet(GLUT_ELAPSED_TIME) / 500.0;
    const double a = t*90.0;

    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cam.refresh();

    glPushMatrix();
        glColor3d(1,0,0);
        glTranslatef(cam.x()+cam.lx(),cam.y()+cam.ly(),cam.z()+cam.lz());
        glutSolidSphere(0.006, 16, 16);
    glPopMatrix();

    glPushMatrix();
      samochod.draw();
    glPopMatrix();

    glPushMatrix();
        glColor3d(0.7,0.7,1);
        glRotated(a*0.005,0,1,0);
        glBindTexture(GL_TEXTURE_2D, texture[8]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gluQuadricTexture(quad,1);
        gluSphere(quad,400,30,30);
    glPopMatrix();

    glColor3f(1,1,1);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBegin(GL_QUADS);
/*    glTexCoord2f(-5.0f, -5.0f); glVertex3f(-100.0f, -100.0f, 0.0f);
    glTexCoord2f(-5.0f, 5.0f); glVertex3f(-100.0f, 100.0f, 0.0f);
    glTexCoord2f(5.0f, 5.0f); glVertex3f(100.0f, 100.0f, 0.0f);
    glTexCoord2f(5.0f, -5.0f); glVertex3f(100.0f, -100.0f, 0.0);*/
        glColor3d(1,1,1);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-100.0f, -100.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-100.0f, 100.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(100.0f, 100.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(100.0f, -100.0f, 0.0f);
    glEnd();

    bul.drawBulet();
    snow.draw();

    glPushMatrix();
        glTranslatef(0,0,50);
        drawSpace(a);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(40,-30,0);
        glRotatef(90,1,0,0);
        drawEgg();
    glPopMatrix();
    glutSwapBuffers();

}

static void key(unsigned char key, int x, int y) {
    switch (key){
        case 27 : case 'q': exit(0); break;
        case '+': slices++; stacks++; break;
        case '-': if (slices>3 && stacks>3){ slices--; stacks--; }break;
        case 'w': cam.keyMove() = 1.0; break;
        case 's': cam.keyMove() = -1.0; break;
        case 'a': cam.keyMoveS() = -1.0; break;
        case 'd': cam.keyMoveS() = 1.0;; break;
        case 'W': cam.keyMove() = 1.0; break;
        case 'S': cam.keyMove() = -1.0; break;
        case 'A': cam.keyMoveS() = -1.0; break;
        case 'D': cam.keyMoveS() = 1.0;; break;
        case 'g': cam.ghostMode() = true; break;
        case 'n': cam.ghostMode() = false; break;

        //Swiatła
        case 'p': glEnable(GL_LIGHT2); break;
        case '0': glDisable(GL_LIGHT2); break;

        case 'o': glEnable(GL_LIGHT3); break;
        case '9': glDisable(GL_LIGHT3); break;

        case 'i': glEnable(GL_LIGHT0); break;
        case '8': glDisable(GL_LIGHT0); break;
    }
    glutPostRedisplay();
}

void keyRelease(unsigned char key, int x, int y){
    switch (key){
       case 'w': cam.keyMove() = 0.0; break;
       case 's': cam.keyMove() = 0.0; break;
       case 'a': cam.keyMoveS() = 0.0; break;
       case 'd': cam.keyMoveS() = 0.0; break;
       case 'W': cam.keyMove() = 0.0; break;
       case 'S': cam.keyMove() = 0.0; break;
       case 'A': cam.keyMoveS() = 0.0; break;
       case 'D': cam.keyMoveS() = 0.0; break;
    }
    glutPostRedisplay();
}

void specialKeyUp( int key, int x, int y ){
    switch(key){
        case GLUT_KEY_UP: keyPressedUP = false; break;
        case GLUT_KEY_DOWN: keyPressedDOWN = false; break;
        case GLUT_KEY_RIGHT: keyPressedLEFT = false; break;
        case GLUT_KEY_LEFT: keyPressedRIGHT = false; break;
    }
}

void specialKey(int key, int x, int y){
    switch(key){
        case GLUT_KEY_UP: keyPressedUP = true; break;
        case GLUT_KEY_DOWN: keyPressedDOWN = true; break;
        case GLUT_KEY_RIGHT: keyPressedLEFT = true; break;
        case GLUT_KEY_LEFT: keyPressedRIGHT = true; break;
    }
}

static void idle(void) {
    if(cam.keyMove()){ cam.updatePositionKey(g_translation_speed); }
    if(cam.keyMoveS()){ cam.updatePositionKeyS(g_translation_speed); }

    if(keyPressedUP == true){
           samochod.x() +=  carSpeed*cos(samochod.angle());
           samochod.y() +=  carSpeed*sin(samochod.angle());
           samochod.rotate() -= 7;
    }
    if(keyPressedDOWN == true){
        samochod.x() -=  carSpeed*cos(samochod.angle());
        samochod.y() -=  carSpeed*sin(samochod.angle());
        samochod.rotate() += 7;
    }
    if(keyPressedLEFT == true){ samochod.angle() -= 0.05f; }
    if(keyPressedRIGHT == true){ samochod.angle() += 0.05f; }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {

    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
         bul.setPosition(cam.x()+cam.lx(),cam.y()+cam.ly(),cam.z()+cam.lz());
         bul.setVector(cam.lx(),cam.ly(),cam.lz());
         bul.draw() = true;
    }
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){ }
    if(state == GLUT_UP){ }
}

void processPassiveMotion(int x, int y) {

    static bool capture = false;

    if(capture) {
       capture = false;
       return;
    }

    int glutX = glutGet(GLUT_SCREEN_WIDTH);
    int glutY = glutGet(GLUT_SCREEN_HEIGHT);

    int dx = x - glutX/2;
    int dy = y - glutY/2;

    if(dx){ cam.rotateYaw(g_rotation_speed*0.5*-dx); }
    if(dy){ cam.rotatePitch(g_rotation_speed*0.5*-dy); }

    glutWarpPointer(glutX/2, glutY/2);

    capture = true;
}

const GLfloat light_ambient[]  = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 10.0f };

int main(int argc, char *argv[]){
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitWindowSize(1366,768);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Michal Kowacki - Projekt");
    glutEnterGameMode(); // Tryb pełnego ekranu
    glEnable(GL_TEXTURE_2D);

    quad = gluNewQuadric();
    Image * earth = loadBMP("earth.bmp");
    texture[0] = loadTexture(earth);

    Image * sun = loadBMP("sun2.bmp");
    texture[1] = loadTexture(sun);

    Image * moon = loadBMP("moon.bmp");
    texture[2] = loadTexture(moon);

    Image * mars = loadBMP("mars.bmp");
    texture[3] = loadTexture(mars);

    Image * mercury = loadBMP("mercury.bmp");
    texture[4] = loadTexture(mercury);

    Image * venus = loadBMP("venus.bmp");
    texture[5] = loadTexture(venus);

    Image * jupiter = loadBMP("jupiter.bmp");
    texture[6] = loadTexture(jupiter);

    Image * saturn = loadBMP("saturn.bmp");
    texture[7] = loadTexture(saturn);

    Image * stars = loadBMP("cstars.bmp");
    texture[8] = loadTexture(stars);

    delete earth;
    delete sun;
    delete moon;
    delete mars;
    delete mercury;
    delete venus;
    delete jupiter;
    delete saturn;
    delete stars;

    glutReshapeFunc(resize);
    glutDisplayFunc(display);

    glutIdleFunc(idle);
    glutTimerFunc(100, drawBulet1, 0);
    glutTimerFunc(100, moveSnowMans, 0);

    glClearColor(0.1,0.1,0.2,0.5);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(key);
    glutKeyboardUpFunc(keyRelease);

    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);

    glutMouseFunc(mouse);
    glutPassiveMotionFunc(processPassiveMotion);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);
    glEnable(GL_LIGHT5);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glutMainLoop();

    return EXIT_SUCCESS;
}

