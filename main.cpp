#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <string.h>
#include "include/irrKlang.h"
#pragma comment(lib, "irrKlang.lib")

#include "glut.h"

using namespace std;
using namespace irrklang;

/* FPS */
int frame = 0, timee, timebase = 0;
float fps_count;
char *fps = new char [100];

/* GAME SETTINGS */
const int window_w = 1300;
const int window_h = 680;
const float map_size = 33.0;
const int ground_size = 33;
const int num_shots = 4;
const int tank_life = 6;
const int tank_energy = 6000;
float tank_speed = 0.07;
float tank_speed_diagonal = 0.06;
float shot_speed = 0.1;

/* DONT CHANGE */
const float front_of_tank = 1.6;
const float back_of_tank = 1.6;
const float left_of_tank = 0.8;
const float right_of_tank = 0.8;
char quote[11][50];

/* OTHER */
static float ratio;
static GLint display_list; // game terrain in list
ISoundEngine* engine = createIrrKlangDevice(); // irrKlang engine
bool buffer[256]; // key presses in buffer
bool settings = false;
int red_status = 0;
int blue_status = 0;


void changeSize(int w, int h) {
	if(h == 0)
		h = 1;
	ratio = ((float)w/2) / h;
}

void buildingWall(float x, float y, float z, int R, int G, int B) {
	glColor3f(R, G+0.3, B);
	glBegin(GL_QUADS);
		// front
		glVertex3f(x, 0, 0);
		glVertex3f(x, 0, z);
		glVertex3f(x, y, z);
		glVertex3f(x, y, 0);
	glEnd();
	glColor3f(R, G+0.3, B);
	glBegin(GL_QUADS);
		// back
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, z);
		glVertex3f(0, y, z);
		glVertex3f(0, y, 0);
	glEnd();
	glColor3f(R, G+0.3, B);
	glBegin(GL_QUADS);
		// right
		glVertex3f(x, 0, z);
		glVertex3f(0, 0, z);
		glVertex3f(0, y, z);
		glVertex3f(x, y, z);
	glEnd();
	glColor3f(R, G+0.3, B);
	glBegin(GL_QUADS);
		// left
		glVertex3f(x, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, y, 0);
		glVertex3f(x, y, 0);
	glEnd();
	glColor3f(R, G, B);
	glBegin(GL_QUADS);
		// top
		glVertex3f(x, y, 0);
		glVertex3f(x, y, z);
		glVertex3f(0, y, z);
		glVertex3f(0, y, 0);
	glEnd();
}

void drawCircle(float x, float y, float z, float radius) {
    glPushMatrix();
		glColor3f(0, 0, 0);
		glRotatef(90, 0, 1, 0);
		glTranslatef(x, y, z);
		static const int circle_points = 100;
		static const float angle = 2.0f * 3.1416f / circle_points;

		glBegin(GL_POLYGON);
			double angle1 = 0.0;
			glVertex2d(radius * cos(0.0) , radius * sin(0.0));
			for (int i = 0; i < circle_points; i++)
			{       
				glVertex2d(radius * cos(angle1), radius *sin(angle1));
				angle1 += angle;
			}
		glEnd();
    glPopMatrix();
}

void addText(int index, float x, float y, float z, float size) {
	int lengthOfQuote = (int)strlen(quote[index]);
	glPushMatrix();
		glTranslatef(x, y, z);
		glScalef(size, size, size);
		glRotatef(180, 0, 1, 0);
		for (int i = 0; i < lengthOfQuote; i++)
		{
			glColor3f(1, 1, 1);
			glutStrokeCharacter(GLUT_STROKE_ROMAN, quote[index][i]);
		}
	glPopMatrix();
}

class Building {
public: 
		int red, green, blue;
		float x_pos, z_pos;
		float x_front_right, x_front_left, x_back_right, x_back_left;
		float z_front_right, z_front_left, z_back_right, z_back_left;
        Building(int R, int G, int B, float x, float z) {
			red = R;
			green = G;
			blue = B;
			x_pos = x;
			z_pos = z;

			z_front_right = z + 9.0;
			x_front_right = x;

			z_back_left = z;
			x_back_left = x + 6.5;

			z_back_right = z;
			x_back_right = x;

			z_front_left = z + 9.0;
			x_front_left = x + 6.5;
		}

		void drawBuilding() {
			glPushMatrix();
				glTranslatef(x_pos, 0, z_pos);
				buildingWall(0.5, 1.0, 9.0, red, green, blue);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(x_pos+6, 0, z_pos);
				buildingWall(0.5, 1.0, 9.0, red, green, blue);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(x_pos+5.0, 0, z_pos);
				buildingWall(1.0, 1.0, 0.5, red, green, blue);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(x_pos+5.0, 0, z_pos+8.5);
				buildingWall(1.0, 1.0, 0.5, red, green, blue);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(x_pos+0.5, 0, z_pos+8.5);
				buildingWall(1.0, 1.0, 0.5, red, green, blue);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(x_pos+0.5, 0, z_pos);
				buildingWall(1.0, 1.0, 0.5, red, green, blue);
			glPopMatrix();
		}
};

class Tank {
public: 
		int red, green, blue, rotate, life, energy;
		float x_pos, z_pos, x_diff, z_diff;
		float x_front_right, x_front_left, x_back_right, x_back_left;
		float z_front_right, z_front_left, z_back_right, z_back_left;
		float tank_slow_speed;
		int time_slow_speed;
		bool destroyed;
        Tank(int R, int G, int B, float x, float z, int rotation, int health, int nrgy, bool destroy) {
			red = R;
			green = G;
			blue = B;
			x_pos = x;
			z_pos = z;
			x_diff = 0;
			z_diff = shot_speed;
			rotate = rotation;
			life = health;
			energy = nrgy;
			destroyed = destroy;
			tank_slow_speed = 0;
			time_slow_speed = 0;
			getTankPosition(front_of_tank, -back_of_tank, -right_of_tank, left_of_tank);
		}

		void drawTank() {
			glPushMatrix();
				glTranslatef(x_pos, 0, z_pos);
				glColor3f(red, green, blue);

				glPushMatrix();
					glRotatef(rotate, 0, 1, 0);
					/* left tracks */
					float x_temp = -0.8;
					for (int i = 0; i < 5; i++) {
						drawCircle(x_temp, 0.22f, 0.76f, 0.13f);
						x_temp += 0.4;
					}
					/* right tracks */
					x_temp = -0.8;
					for (int i = 0; i < 5; i++) {
						drawCircle(x_temp, 0.22f, -0.76f, 0.13f);
						x_temp += 0.4;
					}
					glTranslatef(-0.75, 0, 1.5);
					if (red == 1)
						glColor3f(red - 0.15 , green + 0.3, blue + 0.3);
					else
						glColor3f(red + 0.3 , green + 0.3, blue - 0.15);
					glBegin(GL_QUADS);
						// front
						glVertex3f(0.0f, 0.0f, -0.5f);
						glVertex3f(1.5f, 0.0f, -0.5f);
						glVertex3f(1.5f, 1.0f, 0.0f);
						glVertex3f(0.0f, 1.0f, 0.0f);
						// back
						glVertex3f(0.0f, 0.0f, -2.5f);
						glVertex3f(1.5f, 0.0f, -2.5f);
						glVertex3f(1.5f, 1.0f, -3.0f);
						glVertex3f(0.0f, 1.0f, -3.0f);
					glEnd();
					if (red == 1)
						glColor3f(red , green + 0.3, blue + 0.3);
					else
						glColor3f(red + 0.3 , green + 0.3, blue);
					glBegin(GL_QUADS);
						// right
						glVertex3f(1.5f, 1.0f, 0.0f);
						glVertex3f(1.5f, 0.0f, -0.5f);
						glVertex3f(1.5f, 0.0f, -2.5f);
						glVertex3f(1.5f, 1.0f, -3.0f);
						// left
						glVertex3f(0.0f, 1.0f, 0.0f);
						glVertex3f(0.0f, 0.0f, -0.5f);
						glVertex3f(0.0f, 0.0f, -2.5f);
						glVertex3f(0.0f, 1.0f, -3.0f);
					glEnd();
					glColor3f(red, green, blue);
					glBegin(GL_QUADS);
						// top
						glVertex3f(0.0f, 1.0f, 0.0f);
						glVertex3f(1.5f, 1.0f, 0.0f);
						glVertex3f(1.5f, 1.0f, -3.0f);
						glVertex3f(0.0f, 1.0f, -3.0f);
					glEnd();

					glPushMatrix();
						glTranslatef(0.75, 1, -1.5);
						if (red == 1)
							glColor3f(red , green + 0.3, blue + 0.4);
						else
							glColor3f(red , green + 0.3, blue);
						glutSolidSphere(0.7f, 20, 20);
					glPopMatrix();

					glPushMatrix();
						glColor3f(0, 0, 0);
						GLUquadricObj *quadratic;
						quadratic = gluNewQuadric();
						glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
						glTranslatef(0.75, 1.3, -1.5);
						gluCylinder(quadratic, 0.1f, 0.1f, 2.0f, 32, 32);
					glPopMatrix();
				glPopMatrix();
			glPopMatrix();
		}

		void drawLife() {
			addText(0, this->x_pos + 7.1, 12.6, this->z_pos - 0.3, 0.005);
			float life_percentage = 1.0 - ((float)life/(float)tank_life);
			float show_life = 10 * life_percentage;
			glBegin(GL_QUADS);
				glColor3f(0, 1, 0);
				glVertex3f(this->x_pos + 5, 12.5, this->z_pos-0.3);
				glVertex3f(this->x_pos - 5 + show_life, 12.5, this->z_pos-0.3);
				glVertex3f(this->x_pos - 5 + show_life, 13.0, this->z_pos);
				glVertex3f(this->x_pos + 5, 13.0, this->z_pos);
			glEnd();
		}

		void drawEnergy() {
			addText(1, this->x_pos + 7.4, 11.6, this->z_pos - 0.3, 0.005);
			float energy_percentage = 1.0 - ((float)energy/(float)tank_energy);
			float show_energy = 10.15 * energy_percentage;
			glBegin(GL_QUADS);
				glColor3f(1, 1, 0);
				glVertex3f(this->x_pos + 5.1, 11.5, this->z_pos-0.3);
				glVertex3f(this->x_pos - 5.1 + show_energy, 11.5, this->z_pos-0.3);
				glVertex3f(this->x_pos - 5.1 + show_energy, 12.0, this->z_pos);
				glVertex3f(this->x_pos + 5.1, 12.0, this->z_pos);
			glEnd();
		}

		void moveTankX(float move) {
			if (move > 0) {
				move += tank_slow_speed;
				if (rotate != 90)
					rotate = 90;
				else {
					if (x_pos + move + front_of_tank < map_size)
						x_pos += move;
				}
				z_diff = 0;
				x_diff = shot_speed;
				
				z_front_right = z_pos + right_of_tank;
				x_front_right = x_pos + front_of_tank;

				z_front_left = z_pos - left_of_tank;
				x_front_left = x_pos + front_of_tank;

				z_back_right = z_pos + right_of_tank;
				x_back_right = x_pos - back_of_tank;

				z_back_left = z_pos - left_of_tank;
				x_back_left = x_pos - back_of_tank;
			}
			else {
				move -= tank_slow_speed;
				if (rotate != 270)
					rotate = 270;
				else {
					if (x_pos + move - front_of_tank > -map_size)
						x_pos += move;
				}
				z_diff = 0;
				x_diff = -shot_speed;
				
				z_front_right = z_pos - right_of_tank;
				x_front_right = x_pos - front_of_tank;

				z_front_left = z_pos + left_of_tank;
				x_front_left = x_pos - front_of_tank;

				z_back_right = z_pos - right_of_tank;
				x_back_right = x_pos + back_of_tank;

				z_back_left = z_pos + left_of_tank;
				x_back_left = x_pos + back_of_tank;
			}
			energy--;
			time_slow_speed++;
		}

		void moveTankZ(float move) {		
			if (move > 0) {
				move += tank_slow_speed;
				if (rotate != 0)
					rotate = 0;
				else {
					if (z_pos + move + front_of_tank < map_size)
						z_pos += move;
				}
				z_diff = shot_speed;
				x_diff = 0;
				getTankPosition(front_of_tank, -back_of_tank, -right_of_tank, left_of_tank);
			}
			else {
				move -= tank_slow_speed;
				if (rotate != -180)
					rotate = -180;
				else {
					if (z_pos + move - front_of_tank > -map_size)
						z_pos += move;
				}
				z_diff = -shot_speed;
				x_diff = 0;
				getTankPosition(-front_of_tank, back_of_tank, right_of_tank, -left_of_tank);
			}
			energy--;
			time_slow_speed++;
		}

		void moveUpLeft(float move, int angle, int z_sign, int x_sign) {
			move += tank_slow_speed;
			rotate = angle;

			if (z_pos + move + front_of_tank < map_size && x_pos + move + front_of_tank < map_size) {
				z_pos += move;
				x_pos += move;
			}

			z_diff = shot_speed * z_sign;
			x_diff = shot_speed * x_sign;

			z_front_right = z_pos + front_of_tank;
			x_front_right = x_pos + 0.5;

			z_front_left = z_pos + 0.5;
			x_front_left = x_pos + 1.6;
			
			z_back_right = z_pos - 0.5;
			x_back_right = x_pos - 1.6;

			z_back_left = z_pos - front_of_tank;
			x_back_left = x_pos - 0.5;

			energy--;
			time_slow_speed++;
		}

		void moveDownRight(float move, int angle, int z_sign, int x_sign) {
			move += tank_slow_speed;
			rotate = angle;

			if (z_pos - move - front_of_tank > -map_size && x_pos - move - front_of_tank > -map_size) {
				z_pos -= move;
				x_pos -= move;
			}

			z_diff = shot_speed * z_sign;
			x_diff = shot_speed * x_sign;

			z_front_right = z_pos - front_of_tank;
			x_front_right = x_pos - 0.5;

			z_front_left = z_pos - 0.5;
			x_front_left = x_pos - 1.6;
			
			z_back_right = z_pos + 0.5;
			x_back_right = x_pos + 1.6;

			z_back_left = z_pos + front_of_tank;
			x_back_left = x_pos + 0.5;

			energy--;
			time_slow_speed++;
		}

		void moveUpRight(float move, int angle, int z_sign, int x_sign) {
			move += tank_slow_speed;
			rotate = angle;

			if (z_pos + move + front_of_tank < map_size && x_pos - move - front_of_tank > -map_size) {
				z_pos += move;
				x_pos -= move;
			}
			
			z_diff = shot_speed * z_sign;
			x_diff = shot_speed * x_sign;
			
			z_front_right = z_pos + 0.5;
			x_front_right = x_pos - 1.6;

			z_front_left = z_pos + front_of_tank;
			x_front_left = x_pos - 0.5;
			
			z_back_right = z_pos - front_of_tank;
			x_back_right = x_pos + 0.5;

			z_back_left = z_pos - 0.5;
			x_back_left = x_pos + 1.6;

			energy--;
			time_slow_speed++;
		}

		void moveDownLeft(float move, int angle, int z_sign, int x_sign) {
			move += tank_slow_speed;
			rotate = angle;

			if (z_pos - move - front_of_tank > -map_size && x_pos + move + front_of_tank < map_size) {
				z_pos -= move;
				x_pos += move;
			}

			z_diff = shot_speed * z_sign;
			x_diff = shot_speed * x_sign;

			z_front_right = z_pos - 0.5;
			x_front_right = x_pos + 1.6;

			z_front_left = z_pos - front_of_tank;
			x_front_left = x_pos + 0.5;
			
			z_back_right = z_pos + front_of_tank;
			x_back_right = x_pos - 0.5;

			z_back_left = z_pos + 0.5;
			x_back_left = x_pos - 1.6;

			energy--;
			time_slow_speed++;
		}

		void minusLife() {
			life--;
		}

		void minusEnergy() {
			energy -= 40;
		}

		void healTank() {
			if (life < tank_life)
				life++;
			if (energy < tank_energy)
				energy += 2;
		}

		void getTankPosition(float front, float back, float right, float left) {
			z_front_right = z_pos + front;
			x_front_right = x_pos + right;

			z_front_left = z_pos + front;
			x_front_left = x_pos + left;

			z_back_right = z_pos + back;
			x_back_right = x_pos + right;

			z_back_left = z_pos + back;
			x_back_left = x_pos + left;
		}

		void getTankPositionFrontRight() {
			/*
			float AB = front_of_tank;
			float AC = front_of_tank;

			float v1 = (cos((double)rotate)*(0) + sin((double)rotate)*(z_pos + front_of_tank - z_pos)) / (AB);
			float v2 = (cos((double)rotate)*(z_pos + front_of_tank - z_pos) - sin((double)rotate)*(0)) / (AB);
			x_front_right = x_pos + AC*v1;
			z_front_right = z_pos + AC*v2;
			*/
			//NewPos = new Vector2(825 * sin(85 degrees), 825 * cos(85 degrees));
			x_front_right = x_pos + front_of_tank * sin((float)rotate);
			z_front_right = z_pos + front_of_tank * cos((float)rotate);

			x_front_right = x_front_right + right_of_tank * sin(180.0-rotate);
			z_front_right = z_front_right + right_of_tank * cos(180.0-rotate);
		}

		void getTankPositionFrontLeft() {
			/*
			float AB = front_of_tank;
			float AC = front_of_tank;

			float v1 = (cos((double)rotate)*(0) + sin((double)rotate)*(z_pos + front_of_tank - z_pos)) / (AB);
			float v2 = (cos((double)rotate)*(z_pos + front_of_tank - z_pos) - sin((double)rotate)*(0)) / (AB);
			x_front_left = x_pos + AC*v1;
			z_front_left = z_pos + AC*v2;
			*/
			//x_front_right = x_pos - front_of_tank * sin((float)rotate);
			//z_front_right = z_pos + front_of_tank * cos((float)rotate);
		}

		void setSlowSpeed(float spd) {
			tank_slow_speed = spd;
		}

		void setTimeSlowSpeed(int time) {
			time_slow_speed = time;
		}
};

class Shot {
public: 
		float x_pos, z_pos, x_diff, z_diff;
        Shot(float x, float z, float x_plus, float z_plus) {
			x_pos = x;
			z_pos = z;
			x_diff = x_plus;
			z_diff = z_plus;
		}

		void drawShot() {
			x_pos += x_diff;
			z_pos += z_diff;
			glPushMatrix();
				glTranslatef(x_pos, 0, z_pos);
				glPushMatrix();
					glTranslatef(0, 1.3, 0);
					glColor3f(1, 1, 0);
					glutSolidSphere(0.1f, 20, 20);
				glPopMatrix();
			glPopMatrix();
		}
};

class Explosion {
public:
		float x_pos, z_pos, radius;
		Explosion(float x, float z) {
			x_pos = x;
			z_pos = z;
			radius = 0.8f;
		}

		void drawExplosion() {
			if (radius < 2.5) {
				radius += 0.006;
				glPushMatrix();
					glTranslatef(x_pos, 0, z_pos);
					glColor3f(1, 1, 0);
					glutSolidSphere(radius, 20, 20);
				glPopMatrix();
			}
		}

		void changeRadius(float r) {
			radius = r;
		}
};

class Ground {
public:
		float x_pos, z_pos;
		Ground(float x, float z) {
			x_pos = x+1.5;
			z_pos = z+1.5;
		}

		void drawGround() {
			glColor3f(0, 0.9, 0);
			glPushMatrix();
				glTranslatef(x_pos, 0, z_pos);
				
				glBegin(GL_QUADS);
					// front
					glVertex3f(-1.5, 0, -1.5);
					glVertex3f(-1.5, 0, 1.5);
					glVertex3f(-1.5, 1.7, 1.5);
					glVertex3f(-1.5, 1.7, -1.5);
				glEnd();
				
				glBegin(GL_QUADS);
					// back
					glVertex3f(1.5, 0, 1.5);
					glVertex3f(-1.5, 0, 1.5);
					glVertex3f(-1.5, 1.7, 1.5);
					glVertex3f(1.5, 1.7, 1.5);
				glEnd();
				glBegin(GL_QUADS);
					// left
					glVertex3f(1.5, 0, -1.5);
					glVertex3f(1.5, 0, 1.5);
					glVertex3f(1.5, 1.7, 1.5);
					glVertex3f(1.5, 1.7, -1.5);
				glEnd();
				glBegin(GL_QUADS);
					// right
					glVertex3f(1.5, 0, -1.5);
					glVertex3f(-1.5, 0, -1.5);
					glVertex3f(-1.5, 1.7, -1.5);
					glVertex3f(1.5, 1.7, -1.5);
				glEnd();
			glPopMatrix();
			glColor3f(0, 0.6, 0);
			glPushMatrix();
				glTranslatef(x_pos, 0, z_pos);
				glBegin(GL_QUADS);
					// top
					glVertex3f(1.5, 1.7, 1.5);
					glVertex3f(-1.5, 1.7, 1.5);
					glVertex3f(-1.5, 1.7, -1.5);
					glVertex3f(1.5, 1.7, -1.5);
				glEnd();
			glPopMatrix();
			
		}
};

Building* blue_building;
Building* red_building;

Tank* blue_tank;
Tank* red_tank;

Ground* ground[(ground_size/3)*2][(ground_size/3)*2];

Shot* blue_shots[num_shots];
Shot* red_shots[num_shots];

Explosion* red_explosion = new Explosion(1000, 1000);
Explosion* blue_explosion = new Explosion(1000, 1000);

GLuint createDL() {
	GLuint newDL;
	newDL = glGenLists(1);
	glNewList(newDL,GL_COMPILE);
		glColor3f(0.000, 0.4, 0.000);
		glBegin(GL_QUADS);
			glVertex3f(-map_size, 0.0f, -map_size);
			glVertex3f(-map_size, 0.0f,  map_size);
			glVertex3f( map_size, 0.0f,  map_size);
			glVertex3f( map_size, 0.0f, -map_size);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.8, 0.5, 0.000);
		glBegin(GL_QUADS);
			glVertex3f(map_size, 0.0f, -map_size);
			glVertex3f(-map_size, 0.0f,  -map_size);
			glVertex3f(-map_size, -5.0f,  -map_size);
			glVertex3f(map_size, -5.0f, -map_size);
		glEnd();

		//buildings
		blue_building->drawBuilding();
		red_building->drawBuilding();

		//ground
		for (int i = 0; i < (ground_size/3)*2; i++) {
			for (int j = 0; j < (ground_size/3)*2; j++) {
				if (ground[i][j] != NULL)
					ground[i][j]->drawGround();
			}
		}
	glEndList();

	return(newDL);
}

void clearGroundAroundBuilding(Building* building) {
	if (building->x_pos > 0 && building->z_pos > 0) {
		int z_i = ((int)(building->z_back_right)/3)+(ground_size/3);
		int x_j = ((int)(building->x_back_right)/3)+(ground_size/3);
		for (int i = z_i; i < z_i+4; i++) {
			for (int j = x_j; j < x_j+3; j++) {
				ground[j][i] = NULL;
			}
		}
	}
	else if (building->x_pos > 0 && building->z_pos < 0) {
		int z_i = (((int)(building->z_back_right)/3)+(ground_size/3))-1;
		int x_j = (((int)(building->x_back_right)/3)+(ground_size/3));
		for (int i = z_i; i < z_i+4; i++) {
			for (int j = x_j; j < x_j+3; j++) {
				ground[j][i] = NULL;
			}
		}
	}
	else if (building->x_pos < 0 && building->z_pos < 0) {
		int z_i = (((int)(building->z_back_right)/3)+(ground_size/3))-1;
		int x_j = (((int)(building->x_back_right)/3)+(ground_size/3))-1;
		for (int i = z_i; i < z_i+4; i++) {
			for (int j = x_j; j < x_j+3; j++) {
				ground[j][i] = NULL;
			}
		}
	}
	else if (building->x_pos < 0 && building->z_pos > 0) {
		int z_i = (((int)(building->z_back_right)/3)+(ground_size/3));
		int x_j = (((int)(building->x_back_right)/3)+(ground_size/3))-1;
		for (int i = z_i; i < z_i+4; i++) {
			for (int j = x_j; j < x_j+3; j++) {
				ground[j][i] = NULL;
			}
		}
	}
}

void randomPosition() {
	srand (time(NULL));
	int x_blue = rand() % (((int)map_size*2)-7) - (int)map_size;
	int z_blue = rand() % (((int)map_size*2)-10) - (int)map_size;
	if (x_blue == 0)
		x_blue++;
	if (z_blue == 0)
		z_blue++;
	blue_building = new Building(0, 0, 1, x_blue, z_blue);
	blue_tank = new Tank(0, 0, 1, x_blue+3, z_blue+3, 0, tank_life, tank_energy, false);
	int x_red = rand() % (((int)map_size*2)-7) - (int)map_size;
	int z_red = rand() % (((int)map_size*2)-10) - (int)map_size;
	if (x_red == 0)
		x_red++;
	if (z_red == 0)
		z_red++;
	if (abs(z_red - z_blue) < 10 && abs(x_red - x_blue) < 7) {
		if (x_blue - 10 > -map_size) {
			red_building = new Building(1, 0, 0, x_blue-10, z_red);
			red_tank = new Tank(1, 0, 0, x_blue-7, z_red+3, 0, tank_life, tank_energy, false);
		}
		else {
			red_building = new Building(1, 0, 0, x_blue+10, z_blue-10);
			red_tank = new Tank(1, 0, 0, x_blue+13, z_red+3, 0, tank_life, tank_energy, false);
		}
	}
	else {
		red_building = new Building(1, 0, 0, x_red, z_red);
		red_tank = new Tank(1, 0, 0, x_red+3, z_red+3, 0, tank_life, tank_energy, false);
	}
	for (int i = 0; i < (ground_size/3)*2; i++) {
		for (int j = 0; j < (ground_size/3)*2; j++) {
			ground[i][j] = (Ground*) malloc(sizeof(Ground));
			ground[i][j] = new Ground((i-(ground_size/3))*3, (j-(ground_size/3))*3);
		}
	}

	clearGroundAroundBuilding(red_building);
	clearGroundAroundBuilding(blue_building);

	display_list = createDL();
}

void showStatus(Tank* tank, int status) {
	strcpy(quote[9], "WON: ");
	stringstream strs;
	strs << status;
	string temp_str = strs.str();
	char const* tchar = temp_str.c_str();
	strcat(quote[9], tchar);
	addText(9, tank->x_pos+1.2, 3.0, tank->z_pos, 0.005);
}

bool checkIfTankInsideBuilding(Tank* tank, Building* building) {
	if (tank->x_pos < building->x_front_left && tank->x_pos > building->x_front_right && tank->z_pos < building->z_front_right && tank->z_pos > building->z_back_right)
		return true;
	else
		return false;
}

bool checkIfPointInsideBuilding(Tank* moving_tank, float x, float z, float x_speed, float z_speed) {
	float x_moving_tank = x + x_speed;
	float z_moving_tank = z + z_speed;

	if (z_moving_tank < red_building->z_front_right && z_moving_tank > red_building->z_back_right && x_moving_tank < red_building->x_back_right+0.5 && x_moving_tank > red_building->x_back_right) // red_building right 
		return true;
	else if (z_moving_tank < red_building->z_front_left && z_moving_tank > red_building->z_back_left && x_moving_tank < red_building->x_back_left && x_moving_tank > red_building->x_back_left-0.5) // red_building left 
		return true;
	else if (z_moving_tank < red_building->z_front_right && z_moving_tank > red_building->z_back_right+8.0 && x_moving_tank < red_building->x_back_right+1.6 && x_moving_tank > red_building->x_back_right) // red_building right up
		return true;
	else if (z_moving_tank < red_building->z_front_right-8.0 && z_moving_tank > red_building->z_back_right && x_moving_tank < red_building->x_back_right+1.6 && x_moving_tank > red_building->x_back_right) // red_building right down
		return true;
	else if (z_moving_tank < red_building->z_front_left && z_moving_tank > red_building->z_back_left+8.0 && x_moving_tank < red_building->x_back_left && x_moving_tank > red_building->x_back_left-1.6) // red_building left up
		return true;
	else if (z_moving_tank < red_building->z_front_left-8.0 && z_moving_tank > red_building->z_back_left && x_moving_tank < red_building->x_back_left && x_moving_tank > red_building->x_back_left-1.6) // red_building left down
		return true;
	else if (z_moving_tank < blue_building->z_front_right && z_moving_tank > blue_building->z_back_right && x_moving_tank < blue_building->x_back_right+0.5 && x_moving_tank > blue_building->x_back_right) // blue_building right 
		return true;
	else if (z_moving_tank < blue_building->z_front_left && z_moving_tank > blue_building->z_back_left && x_moving_tank < blue_building->x_back_left && x_moving_tank > blue_building->x_back_left-0.5) // blue_building left 
		return true;
	else if (z_moving_tank < blue_building->z_front_right && z_moving_tank > blue_building->z_back_right+8.0 && x_moving_tank < blue_building->x_back_right+1.6 && x_moving_tank > blue_building->x_back_right) // red_building right up
		return true;
	else if (z_moving_tank < blue_building->z_front_right-8.0 && z_moving_tank > blue_building->z_back_right && x_moving_tank < blue_building->x_back_right+1.6 && x_moving_tank > blue_building->x_back_right) // red_building right down
		return true;
	else if (z_moving_tank < blue_building->z_front_left && z_moving_tank > blue_building->z_back_left+8.0 && x_moving_tank < blue_building->x_back_left && x_moving_tank > blue_building->x_back_left-1.6) // red_building left up
		return true;
	else if (z_moving_tank < blue_building->z_front_left-8.0 && z_moving_tank > blue_building->z_back_left && x_moving_tank < blue_building->x_back_left && x_moving_tank > blue_building->x_back_left-1.6) // red_building left down
		return true;
	else 
		return false;
}

bool checkIfPointInsideArea(Tank* moving_tank, Tank* tank, float x, float z, float x_speed, float z_speed) {
	float x_moving_tank = x + x_speed;
	float z_moving_tank = z + z_speed;

	float a1 = sqrt( pow(tank->x_front_left - tank->x_front_right, 2) + pow(tank->z_front_left - tank->z_front_right, 2) );
	float a2 = sqrt( pow(tank->x_front_right - tank->x_back_right, 2) + pow(tank->z_front_right - tank->z_back_right, 2) );
	float a3 = sqrt( pow(tank->x_back_right - tank->x_back_left, 2) + pow(tank->z_back_right - tank->z_back_left, 2) );
	float a4 = sqrt( pow(tank->x_back_left - tank->x_front_left, 2) + pow(tank->z_back_left - tank->z_front_left, 2) );

	float b1 = sqrt( pow(tank->x_front_left - x_moving_tank, 2) + pow(tank->z_front_left - z_moving_tank, 2) );
	float b2 = sqrt( pow(tank->x_front_right - x_moving_tank, 2) + pow(tank->z_front_right - z_moving_tank, 2) );
	float b3 = sqrt( pow(tank->x_back_left - x_moving_tank, 2) + pow(tank->z_back_left - z_moving_tank, 2) );
	float b4 = sqrt( pow(tank->x_back_right - x_moving_tank, 2) + pow(tank->z_back_right - z_moving_tank, 2) );

	float u1 = (a1 + b1 + b2)/2;
	float u2 = (a2 + b2 + b3)/2;
	float u3 = (a3 + b3 + b4)/2;
	float u4 = (a4 + b4 + b1)/2;

	float volumeA = abs(( u1*(u1 - a1)*(u1 - b1)*(u1 - b2) )/2);
	float volumeB = abs(( u2*(u2 - a2)*(u2 - b2)*(u2 - b3) )/2);
	float volumeC = abs(( u3*(u3 - a3)*(u3 - b3)*(u3 - b4) )/2);
	float volumeD = abs(( u4*(u4 - a4)*(u4 - b4)*(u4 - b1) )/2);
	
	float volumeRectangle = a1 * a2;

	if (volumeRectangle > volumeA + volumeB + volumeC + volumeD)
		return true;
	else
		return false;
}

void redTankMoving() {
	if (red_tank->tank_slow_speed < 0 && red_tank->time_slow_speed > 50) {
		red_tank->setSlowSpeed(0);
		red_tank->setTimeSlowSpeed(0);
	}
	if (buffer['w'] && buffer['a']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, 0, tank_speed))
			red_tank->moveUpLeft(tank_speed_diagonal, -315, 1, 1);
		else
			red_tank->moveDownRight(tank_speed_diagonal, -315, 1, 1);
		return;
	}
	if (buffer['w'] && buffer['d']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, -tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, -tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, -tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, -tank_speed_diagonal, tank_speed_diagonal))
			red_tank->moveUpRight(tank_speed_diagonal, -45, 1, -1);
		else
			red_tank->moveDownLeft(tank_speed_diagonal, -45, 1, -1);
		return;
	}
	if (buffer['a'] && buffer['s']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, tank_speed_diagonal, -tank_speed_diagonal))
			red_tank->moveDownLeft(tank_speed_diagonal, -225, -1, 1);
		else
			red_tank->moveUpRight(tank_speed_diagonal, -225, -1, 1);
		return;
	}
	if (buffer['s'] && buffer['d']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, -tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, -tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, -tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, -tank_speed_diagonal, -tank_speed_diagonal))
			red_tank->moveDownRight(tank_speed_diagonal, -135, -1, -1);
		else
			red_tank->moveUpLeft(tank_speed_diagonal, -135, -1, -1);
		return;
	}
	
	if (buffer['w']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, 0, tank_speed) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, 0, tank_speed) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, 0, tank_speed) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, 0, tank_speed))
			red_tank->moveTankZ(tank_speed);
	}
	if (buffer['s']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, 0, -tank_speed) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, 0, -tank_speed) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, 0, -tank_speed) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, 0, -tank_speed))
			red_tank->moveTankZ(-tank_speed);
	}
	if (buffer['a']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, tank_speed, 0) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, tank_speed, 0) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, tank_speed, 0) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, tank_speed, 0))
			red_tank->moveTankX(tank_speed);
	}
	if (buffer['d']) {
		if (!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_left, red_tank->z_front_left, -tank_speed, 0) &&
			!checkIfPointInsideArea(red_tank, blue_tank, red_tank->x_front_right, red_tank->z_front_right, -tank_speed, 0) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_left, red_tank->z_front_left, -tank_speed, 0) &&
			!checkIfPointInsideBuilding(red_tank, red_tank->x_front_right, red_tank->z_front_right, -tank_speed, 0))
			red_tank->moveTankX(-tank_speed);
	}
}

void blueTankMoving() {
	if (blue_tank->tank_slow_speed < 0 && blue_tank->time_slow_speed > 50) {
		blue_tank->setSlowSpeed(0);
		blue_tank->setTimeSlowSpeed(0);
	}
	if (buffer['i'] && buffer['j']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, tank_speed_diagonal, tank_speed_diagonal))
			blue_tank->moveUpLeft(tank_speed_diagonal, -315, 1, 1);
		else
			blue_tank->moveDownRight(tank_speed_diagonal, -315, 1, 1);
		return;
	}
	if (buffer['i'] && buffer['l']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, -tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, -tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, -tank_speed_diagonal, tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, -tank_speed_diagonal, tank_speed_diagonal))
			blue_tank->moveUpRight(tank_speed_diagonal, -45, 1, -1);
		else
			blue_tank->moveDownLeft(tank_speed_diagonal, -45, 1, -1);
		return;
	}
	if (buffer['j'] && buffer['k']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, tank_speed_diagonal, -tank_speed_diagonal))
			blue_tank->moveDownLeft(tank_speed_diagonal, -225, -1, 1);
		else
			blue_tank->moveUpRight(tank_speed_diagonal, -225, -1, 1);
		return;
	}
	if (buffer['k'] && buffer['l']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, -tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, -tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, -tank_speed_diagonal, -tank_speed_diagonal) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, -tank_speed_diagonal, -tank_speed_diagonal))
			blue_tank->moveDownRight(tank_speed_diagonal, -135, -1, -1);
		else
			blue_tank->moveUpLeft(tank_speed_diagonal, -135, -1, -1);
		return;
	}
	
	if (buffer['i']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, 0, tank_speed) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, 0, tank_speed) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, 0, tank_speed) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, 0, tank_speed))
			blue_tank->moveTankZ(tank_speed);
	}
	if (buffer['k']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, 0, -tank_speed) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, 0, -tank_speed) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, 0, -tank_speed) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, 0, -tank_speed))
			blue_tank->moveTankZ(-tank_speed);
	}
	if (buffer['j']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, tank_speed, 0) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, tank_speed, 0) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, tank_speed, 0) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, tank_speed, 0))
			blue_tank->moveTankX(tank_speed);
	}
	if (buffer['l']) {
		if (!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_left, blue_tank->z_front_left, -tank_speed, 0) &&
			!checkIfPointInsideArea(blue_tank, red_tank, blue_tank->x_front_right, blue_tank->z_front_right, -tank_speed, 0) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_left, blue_tank->z_front_left, -tank_speed, 0) &&
			!checkIfPointInsideBuilding(blue_tank, blue_tank->x_front_right, blue_tank->z_front_right, -tank_speed, 0))
			blue_tank->moveTankX(-tank_speed);
	}
}

void keyboardOperations() {
	if (red_tank->life > 0 && blue_tank->life > 0) {
		redTankMoving();
		blueTankMoving();
	}
}

void keyboardListener(unsigned char key, int x, int y) {
	buffer[key] = true;
	if (red_tank->life > 0 && blue_tank->life > 0) {
		if (key == 'v') {
			for (int i = 0; i < num_shots; i++) {
				if (red_shots[i] == NULL) {
					engine->play2D("shot.wav");
					red_shots[i] = new Shot(red_tank->x_pos, red_tank->z_pos, red_tank->x_diff, red_tank->z_diff);
					red_tank->minusEnergy();
					break;
				}
			}
		}
		if (key == 'n') {
			for (int i = 0; i < num_shots; i++) {
				if (blue_shots[i] == NULL) {
					engine->play2D("shot.wav");
					blue_shots[i] = new Shot(blue_tank->x_pos, blue_tank->z_pos, blue_tank->x_diff, blue_tank->z_diff);
					blue_tank->minusEnergy();
					break;
				}
			}
		}
	}
}

void keyboardListenerUp(unsigned char key, int x, int y) {
	buffer[key] = false;
}

void specialKeyboardListener(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_F1 : 
			engine->drop();
			exit(0);
			break;
		case GLUT_KEY_F2 :
			randomPosition();
			break;
		case GLUT_KEY_F3 :
			if (settings)
				settings = false;
			else
				settings = true;
			break;
	}
}

bool groundShot(Shot* shot) {
	if (shot->x_pos > 0 && shot->z_pos > 0) {
		if (ground[((int)(shot->x_pos)/3)+(ground_size/3)][((int)(shot->z_pos)/3)+(ground_size/3)] != NULL) {
			ground[((int)(shot->x_pos)/3)+(ground_size/3)][((int)(shot->z_pos)/3)+(ground_size/3)] = NULL;
			display_list = createDL();
			return true;
		}
		else 
			return false;
	}
	else if (shot->x_pos > 0 && shot->z_pos < 0) {
		if (ground[((int)(shot->x_pos)/3)+(ground_size/3)][((int)(shot->z_pos)/3)+(ground_size/3)-1] != NULL) {
			ground[((int)(shot->x_pos)/3)+(ground_size/3)][((int)(shot->z_pos)/3)+(ground_size/3)-1] = NULL;
			display_list = createDL();
			return true;
		}
		else 
			return false;
	}
	else if (shot->x_pos < 0 && shot->z_pos < 0) {
		if (ground[((int)(shot->x_pos)/3)+(ground_size/3)-1][((int)(shot->z_pos)/3)+(ground_size/3)-1] != NULL) {
			ground[((int)(shot->x_pos)/3)+(ground_size/3)-1][((int)(shot->z_pos)/3)+(ground_size/3)-1] = NULL;
			display_list = createDL();
			return true;
		}
		else 
			return false;
	}
	else if (shot->x_pos < 0 && shot->z_pos > 0) {
		if (ground[((int)(shot->x_pos)/3)+(ground_size/3)-1][((int)(shot->z_pos)/3)+(ground_size/3)] != NULL) {
			ground[((int)(shot->x_pos)/3)+(ground_size/3)-1][((int)(shot->z_pos)/3)+(ground_size/3)] = NULL;
			display_list = createDL();
			return true;
		}
		else 
			return false;
	}
	return false;
}

void battleOperations(Tank* tank) {
	//terrain
	glCallList(display_list);

	//tanks
	blue_tank->drawTank();
	red_tank->drawTank();

	//tank life
	tank->drawLife();
	//tank energy
	tank->drawEnergy();
	
	if (tank->x_pos > 0 && tank->z_pos > 0) {
		if (ground[((int)(tank->x_pos)/3)+(ground_size/3)][((int)(tank->z_pos)/3)+(ground_size/3)] != NULL) {
			tank->setSlowSpeed(-0.02);
			tank->setTimeSlowSpeed(0);
			ground[((int)(tank->x_pos)/3)+(ground_size/3)][((int)(tank->z_pos)/3)+(ground_size/3)] = NULL;
			display_list = createDL();
		}
	}
	else if (tank->x_pos > 0 && tank->z_pos < 0) {
		if (ground[((int)(tank->x_pos)/3)+(ground_size/3)][((int)(tank->z_pos)/3)+(ground_size/3)-1] != NULL) {
			tank->setSlowSpeed(-0.02);
			tank->setTimeSlowSpeed(0);
			ground[((int)(tank->x_pos)/3)+(ground_size/3)][((int)(tank->z_pos)/3)+(ground_size/3)-1] = NULL;
			display_list = createDL();
		}
	}
	else if (tank->x_pos < 0 && tank->z_pos < 0) {
		if (ground[((int)(tank->x_pos)/3)+(ground_size/3)-1][((int)(tank->z_pos)/3)+(ground_size/3)-1] != NULL) {
			tank->setSlowSpeed(-0.02);
			tank->setTimeSlowSpeed(0);
			ground[((int)(tank->x_pos)/3)+(ground_size/3)-1][((int)(tank->z_pos)/3)+(ground_size/3)-1] = NULL;
			display_list = createDL();
		}
	}
	else if (tank->x_pos < 0 && tank->z_pos > 0) {
		if (ground[((int)(tank->x_pos)/3)+(ground_size/3)-1][((int)(tank->z_pos)/3)+(ground_size/3)] != NULL) {
			tank->setSlowSpeed(-0.02);
			tank->setTimeSlowSpeed(0);
			ground[((int)(tank->x_pos)/3)+(ground_size/3)-1][((int)(tank->z_pos)/3)+(ground_size/3)] = NULL;
			display_list = createDL();
		}
	}

	//shots
	for (int i = 0; i < num_shots; i++) {
		if (blue_shots[i] != NULL) {
			blue_shots[i]->drawShot();
			if (blue_shots[i]->x_pos > map_size || blue_shots[i]->z_pos < -map_size || blue_shots[i]->x_pos < -map_size || blue_shots[i]->z_pos > map_size)
				blue_shots[i] = NULL;
			else if (groundShot(blue_shots[i]))
				blue_shots[i] = NULL;
			else if ((blue_shots[i]->x_pos <= red_tank->x_pos + 0.6 && blue_shots[i]->x_pos >= red_tank->x_pos - 0.6) && (blue_shots[i]->z_pos <= red_tank->z_pos + 0.6 && blue_shots[i]->z_pos >= red_tank->z_pos - 0.6)) {
				blue_shots[i] = NULL;
				red_tank->minusLife();
				if (red_tank->life <= 0) {
					if (!red_tank->destroyed) {
						blue_status++;
					}
					engine->play2D("battle-explosion.wav");
					red_explosion = new Explosion(red_tank->x_pos, red_tank->z_pos);
					red_tank = new Tank(0, 0, 0, red_tank->x_pos, red_tank->z_pos, red_tank->rotate, 0, 50, true);
				}
				else {
					if ((rand() % 2) == 0)
						engine->play2D("enemy-is-hit.ogg");
					else
						engine->play2D("that-one-went-right-through.ogg");
				}
			}
		}
	}
	for (int i = 0; i < num_shots; i++) {
		if (red_shots[i] != NULL) {
			red_shots[i]->drawShot();
			if (red_shots[i]->x_pos > map_size || red_shots[i]->z_pos < -map_size || red_shots[i]->x_pos < -map_size || red_shots[i]->z_pos > map_size)
				red_shots[i] = NULL;
			else if (groundShot(red_shots[i]))
				red_shots[i] = NULL;
			else if ((red_shots[i]->x_pos <= blue_tank->x_pos + 0.6 && red_shots[i]->x_pos >= blue_tank->x_pos - 0.6) && (red_shots[i]->z_pos <= blue_tank->z_pos + 0.6 && red_shots[i]->z_pos >= blue_tank->z_pos - 0.6)) {
				red_shots[i] = NULL;
				blue_tank->minusLife();
				if (blue_tank->life <= 0) {
					if (!blue_tank->destroyed) {
						red_status++;
					}
					engine->play2D("battle-explosion.wav");
					blue_explosion = new Explosion(blue_tank->x_pos, blue_tank->z_pos);
					blue_tank = new Tank(0, 0, 0, blue_tank->x_pos, blue_tank->z_pos, blue_tank->rotate, 0, 50, true);
				}
				else {
					if ((rand() % 2) == 0)
						engine->play2D("enemy-is-hit.ogg");
					else
						engine->play2D("that-one-went-right-through.ogg");
				}
			}
		}
	}

	//check tank life
	if (red_tank->life == 0 && red_tank->energy > 0) {
		red_explosion->drawExplosion();

		showStatus(red_tank, red_status);
		showStatus(blue_tank, blue_status);

		if (red_explosion->radius > 2.5 && red_explosion->radius < 5) {
			engine->play2D("enemy-vehicle-destroyed.wav");
			red_explosion->changeRadius(6.0);
		}
	}
	if (blue_tank->life == 0 && blue_tank->energy > 0) {
		blue_explosion->drawExplosion();
		
		showStatus(red_tank, red_status);
		showStatus(blue_tank, blue_status);

		if (blue_explosion->radius > 2.5 && blue_explosion->radius < 5) {
			engine->play2D("enemy-vehicle-destroyed.wav");
			blue_explosion->changeRadius(6.0);
		}
	}

	//check tank energy
	if (red_tank->energy <= 0) {
		showStatus(red_tank, red_status);
		showStatus(blue_tank, blue_status);
		if (!red_tank->destroyed) {
			blue_status++;
			red_tank = new Tank(0, 0, 0, red_tank->x_pos, red_tank->z_pos, red_tank->rotate, 0, -10, true);
			engine->play2D("bail-out-this-vehicle-has-had-it.ogg");
		}
	}
	if (blue_tank->energy <= 0) {
		showStatus(red_tank, red_status);
		showStatus(blue_tank, blue_status);
		if (!blue_tank->destroyed) {
			red_status++;
			blue_tank = new Tank(0, 0, 0, blue_tank->x_pos, blue_tank->z_pos, blue_tank->rotate, 0, -10, true);
			engine->play2D("bail-out-this-vehicle-has-had-it.ogg");
		}
	}
}

void initScene() {
	glEnable(GL_DEPTH_TEST);

	//FPS COUNT
	frame++;
	timee = glutGet(GLUT_ELAPSED_TIME);
	if (timee - timebase > 1000) {
		fps_count = frame*1000.0 / (timee - timebase);
		sprintf(fps, "FPS:%4.2f", fps_count);
		timebase = timee;
		frame = 0;
		
		if (fps_count < 150) {
			tank_speed = 0.07;
			tank_speed_diagonal = 0.06;
			shot_speed = 0.1;
		}
		else if (fps_count > 150 && fps_count < 200) {
			tank_speed = 0.06;
			tank_speed_diagonal = 0.05;
			shot_speed = 0.09;
		}
		else {
			tank_speed = 0.05;
			tank_speed_diagonal = 0.04;
			shot_speed = 0.08;
		}
		//cout << fps << endl;
	}
}

void renderScene(void) {
	keyboardOperations();

	//heal red tank if inside building
	if (checkIfTankInsideBuilding(red_tank, red_building)) {
		red_tank->healTank();
	}

	//heal blue tank if inside building
	if (checkIfTankInsideBuilding(blue_tank, blue_building)) {
		blue_tank->healTank();
	}

	glEnable(GL_SCISSOR_TEST);

	/* FIRST VIEWPORT */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, window_w/2 - (8), window_h);
	gluPerspective(45, ratio, 1, 2000);
	glMatrixMode(GL_MODELVIEW);
	glScissor(0, 0, window_w/2, window_h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(red_tank->x_pos, 15, red_tank->z_pos - 20, red_tank->x_pos, 0, red_tank->z_pos + 10, 0.0f, 1.0f, 0.0f);

	battleOperations(red_tank);

	addText(2, red_tank->x_pos + 4.8, 13.3, red_tank->z_pos - 0.3, 0.002);

	if (settings) {
		addText(3, red_tank->x_pos + 0.7, 8.6, red_tank->z_pos - 0.3, 0.005);
		addText(4, red_tank->x_pos + 2.1, 7.6, red_tank->z_pos - 0.3, 0.005);
		addText(5, red_tank->x_pos + 2.1, 6.6, red_tank->z_pos - 0.3, 0.005);
	}

	/* SECOND VIEWPORT */
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(window_w/2, 0, window_w/2, window_h);
    gluPerspective(45, ratio, 1, 2000);
    glMatrixMode(GL_MODELVIEW);
    glScissor(window_w/2, 0, window_w/2, window_h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	gluLookAt(blue_tank->x_pos, 15, blue_tank->z_pos - 20, blue_tank->x_pos, 0, blue_tank->z_pos + 10, 0.0f, 1.0f, 0.0f);

	battleOperations(blue_tank);

	if (settings) {
		addText(6, blue_tank->x_pos + 0.7, 8.6, blue_tank->z_pos - 0.3, 0.005);
		addText(7, blue_tank->x_pos + 2.1, 7.6, blue_tank->z_pos - 0.3, 0.005);
		addText(8, blue_tank->x_pos + 2.1, 6.6, blue_tank->z_pos - 0.3, 0.005);
	}

	glDisable(GL_SCISSOR_TEST);
    glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	strcpy(quote[0],"health");
	strcpy(quote[1],"energy");
	strcpy(quote[2],"F1: EXIT   F2: RESTART   F3: SETTINGS");
	strcpy(quote[3],"W");
	strcpy(quote[4],"A  S  D");
	strcpy(quote[5],"SHOOT: V");
	strcpy(quote[6],"I");
	strcpy(quote[7],"J  K  L");
	strcpy(quote[8],"SHOOT: N");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(window_w, window_h);
	glutCreateWindow("Tank Battles");
	glClearColor(0.373,0.620,0.627,1);

	glutSetCursor(GLUT_CURSOR_NONE);
	glutKeyboardFunc(keyboardListener);
	glutKeyboardUpFunc(keyboardListenerUp);
	glutSpecialFunc(specialKeyboardListener);

	randomPosition();
	display_list = createDL();

	glutDisplayFunc(initScene);
	glutIdleFunc(renderScene);

	if (!engine) 
		return 1;
	engine->play2D("battle.ogg", true);

	glutReshapeFunc(changeSize);

	glutMainLoop();

	return(0);
}