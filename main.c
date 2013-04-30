#include <unistd.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>


GLuint font_tex;
GLuint char_base;

void init_font(void) {

	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SDL_Surface* img = IMG_Load("font.png");
	glGenTextures(1, &font_tex);
	glBindTexture(GL_TEXTURE_2D, font_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, img->w, img->h,
		0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_FreeSurface(img);

	float sx = 1. / 8;
	float sy = 1. / 12;

	char_base = glGenLists(128);
	glListBase(char_base);
	int i;
	for(i = 0; i < 128; i++) {

		glNewList(char_base + i, GL_COMPILE);

		int c = i - 32;
		if(c >= 0 && c < 96) {

			int a = c & 7;
			int b = c >> 3;

			float tx = sx * a;
			float ty = sy * b;

			glBegin(GL_QUADS);
			glTexCoord2f(tx, ty);
			glVertex2d(0, 0);
			glTexCoord2f(tx + sx, ty);
			glVertex2d(8, 0);
			glTexCoord2f(tx + sx, ty + sy);
			glVertex2d(8, 8);
			glTexCoord2f(tx, ty + sy);
			glVertex2d(0, 8);
			glEnd();
		}

		glTranslatef(8, 0, 0);
		glEndList();
	}
}
void print(int x, int y, const char* text, ...) {

	char line[256];
	va_list args;
	va_start(args, text);
	vsnprintf(line, 256, text, args);
	va_end(args);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 800, 600, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(x, y, 0);

	glCallLists(strlen(line), GL_BYTE, line);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);

	glDisable(GL_TEXTURE_2D);
}


void render_cube(void) {

	glBegin(GL_QUADS);
	glColor3f(0, 1, 0);
	glVertex3f( 1, 1,-1);
	glVertex3f(-1, 1,-1);
	glVertex3f(-1, 1, 1);
	glVertex3f( 1, 1, 1);

	glColor3f(1, 0.5, 0);
	glVertex3f( 1,-1, 1);
	glVertex3f(-1,-1, 1);
	glVertex3f(-1,-1,-1);
	glVertex3f( 1,-1,-1);

	glColor3f(1, 0, 0);
	glVertex3f( 1, 1, 1);
	glVertex3f(-1, 1, 1);
	glVertex3f(-1,-1, 1);
	glVertex3f( 1,-1, 1);

	glColor3f(1, 1, 0);
	glVertex3f( 1,-1,-1);
	glVertex3f(-1,-1,-1);
	glVertex3f(-1, 1,-1);
	glVertex3f( 1, 1,-1);

	glColor3f(0, 0, 1);
	glVertex3f(-1, 1, 1);
	glVertex3f(-1, 1,-1);
	glVertex3f(-1,-1,-1);
	glVertex3f(-1,-1, 1);

	glColor3f(1, 0, 1);
	glVertex3f( 1, 1,-1);
	glVertex3f( 1, 1, 1);
	glVertex3f( 1,-1, 1);
	glVertex3f( 1,-1,-1);
	glEnd();

}

void GetEulerAngles(float q0,float q1, float q2, float q3, float* yaw, float* pitch, float* roll)
{
	const double w2 = q0*q0;
	const double x2 = q1*q1;
	const double y2 = q2*q2;
	const double z2 = q3*q3;
	const double unitLength = w2 + x2 + y2 + z2;    // Normalised == 1, otherwise correction divisor.
	const double abcd = q0*q1 + q2*q3;
	const double eps = 1e-7;    // TODO: pick from your math lib instead of hardcoding.
	const double pi = 3.14159265358979323846;   // TODO: pick from your math lib instead of hardcoding.
	if (abcd > (0.5-eps)*unitLength)
	{
		*yaw = 2 * atan2(q2,q0);
		*pitch = pi;
		*roll = 0;
	}
	else if (abcd < (-0.5+eps)*unitLength)
	{
		*yaw = -2 * atan2(q2,q0);
		*pitch = -pi;
		*roll = 0;
	}
	else
	{
		const double adbc = q0*q3 - q1*q2;
		const double acbd = q0*q2 - q1*q3;
		*yaw = atan2(2*adbc, 1 - 2*(z2+x2));
		*pitch = asin(2*abcd/unitLength);
		*roll = atan2(2*acbd, 1 - 2*(y2+x2));
	}
}



void GetAngles(float q0,float q1, float q2,float q3,float * angle, float * x,float * y,float * z) {
	//	if (q0 > 1) q1.normalise(); // if w>1 acos and sqrt will produce errors, this cant happen if quaternion is normalised
	*angle = 2 * acos(q0);
	double s = sqrt(1-(q0*q0)); // assuming quaternion normalised then w is less than 1, so term always positive.
	if (s < 0.001) { // test to avoid divide by zero, s is always positive due to sqrt
		// if s close to zero then direction of axis not important
		*x = q1; // if it is important that axis is normalised then replace with x=1; y=z=0;
		*y = q2;
		*z = q3;
	} else {
		*x = q1 / s; // normalise axis
		*y = q2 / s;
		*z = q3 / s;
	}
}



GLuint load_model(const char* filename) {
	FILE* f = fopen(filename, "rt");
	if (!f) return 0;

	int vc, pc;
	fscanf(f, "%d %d", &vc, &pc);

	float* v = (float*) malloc(vc * sizeof(float) * 6);
	for (int i = 0; i < vc * 6; i += 6) {
		// X Z Y
		fscanf(f, "%f %f %f %f %f %f\n", &v[i], &v[i + 2], &v[i + 1],&v[i + 3],&v[i + 4],&v[i + 5]);
	}
	

	GLuint list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glColor3f(0.7, 0.7, 1);

	while (pc--) {
		int i[32];		// overflow is 'unlikely' ^_^
		int ic = 0;
		char line[256];
		char* m = fgets(line, 256, f);
		do i[ic++] = atoi(m);
		while ((m = strchr(m+1, ' ')));

		float* a = &v[i[0] * 6];
		float* b = &v[i[1] * 6];
		float* c = &v[i[2] * 6];
		float d[3] = { c[0]-a[0], c[1]-a[1], c[2]-a[2] };
		float e[3] = { b[0]-a[0], b[1]-a[1], b[2]-a[2] };
		float n[3] = {
			d[1]*e[2]-d[2]*e[1],
			d[2]*e[0]-d[0]*e[2],
			d[0]*e[1]-d[1]*e[0]
		};
		float l = 1.0 / sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		glNormal3f(n[0] * l, n[1] * l, n[2] * l);

		glBegin(GL_TRIANGLE_FAN);
		glColor3f(v[pc * 6 + 3], v[pc * 6 + 4], v[pc * 6 + 5]);
		for (int j = 0; j < ic; j++) {
			glVertex3f(	v[i[j] * 6 + 0] * 0.5,
						v[i[j] * 6 + 1] * 0.5,
						v[i[j] * 6 + 2] * 0.5);
		}
		glEnd();

	}
	glColor3f(1, 1, 1);
	glEndList();
	fclose(f);
	free(v);
	return list;
}

int main(int argc, char** argv) {


	SDL_Init(SDL_INIT_VIDEO);
	SDL_Surface *surface = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);

	glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	glEnable(GL_MULTISAMPLE);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(25, 800.0 / 600.0, 0.6, 100);
	glMatrixMode(GL_MODELVIEW);

	init_font();

	FILE *fp;
	fp=fopen("sample.log", "r");
	//fp=fopen("/dev/cu.usbmodem1", "r");


	GLuint ship = load_model("ship.model");

	float tick_diff = 0.0f;
	int running = 1;
	int pause = 0;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = 0;
							break;
						case SDLK_SPACE:
							if(pause)
							{
								pause = 0;
							}
							else
							{
								pause = 1;
							}
							break;
						default: break;
					}
			}
		}

		if(pause)
		{
			continue;
		}

		float tick=0;
		float q0=0;
		float q1=0;
		float q2=0;
		float q3=0;
		float acc_x=0;
		float acc_y=0;
		float acc_z=0;
		float gyro_x=0;
		float gyro_y=0;
		float gyro_z=0;

		int i = fscanf(fp,"[%f] %f %f %f %f %f %f %f %f %f %f\n",&tick,&q0,&q1,&q2,&q3,&acc_x,&acc_y,&acc_z,&gyro_x,&gyro_y,&gyro_z);

		if(i==-1)
		{
			SDL_Quit();
			return 0;
		}

		if(tick_diff == 0.0f)
			tick_diff = (tick*1000)-SDL_GetTicks();

		float yaw=0;
		float pitch=0;
		float roll=0;

		GetEulerAngles(q3,q2,q1,q0,&yaw,&pitch,&roll);



		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		glTranslatef(0, 0, -10); //unzoom

		float x,y,z;
		float angle;
		GetAngles(q0,q1,q2,q3,&angle,&x,&y,&z); //sets x,y,z and angle

		angle = angle*180/M_PI;
		glRotatef(angle, y, z, x);
		glRotatef(-90, 0, 1, 0);

		glCallList(ship);
		//render_cube();

		glColor3f(1, 1, 1);
		print(10, 10, "Gyro  X:%5.2f Y:%5.2f Z:%5.2f", gyro_x,gyro_y,gyro_z);
		print(10, 20, "ACC   X:%5.2f Y:%5.2f Z:%5.2f", acc_x,acc_y,acc_z);
		print(10, 50, "Euler P:%5.0f R:%5.0f Y:%5.0f", pitch/M_PI*180,roll/M_PI*180,yaw/M_PI*180);
		print(10, 70, "Timecode %.2f", tick);

		SDL_GL_SwapBuffers();

		Uint32 lastFrame = SDL_GetTicks(); 
		if((((tick*1000)-lastFrame)-tick_diff) > 0)
			SDL_Delay(((tick*1000)-lastFrame)-tick_diff);

	}

	SDL_Quit();

	return 0;
}
