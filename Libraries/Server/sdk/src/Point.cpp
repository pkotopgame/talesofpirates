/* Generated by Together */

#include "Point.h"

void Point::move(short dir,long distance){
	dir %=360;
	if(dir>=0 &&dir <90){
		x +=long(g_sin[dir]*distance+0.01f);			//Testing some changes Mdr.st May 2020 - FPO alpha
		y -=long(g_cos[dir]*distance+0.01f);			// Original: 0.5f
	}else if(dir>= 90 &&dir <180){
		x +=long(g_sin[180-dir]*distance+0.01f);
		y +=long(g_cos[180-dir]*distance+0.01f);
	}else if(dir>=180 &&dir <270){
		x -=long(g_sin[dir-180]*distance+0.01f);
		y +=long(g_cos[dir-180]*distance+0.01f);
	}else if(dir>=270 &&dir <360){
		x -=long(g_sin[360-dir]*distance+0.01f);
		y -=long(g_cos[360-dir]*distance+0.01f);
	}
}
void  eddy(long x,long y,short dir,long radius,long &retx,long &rety)
{
	Point l_p;
	l_p.x	=x;
	l_p.y	=y;
	l_p.move(dir,radius);
	retx	=l_p.x;
	rety	=l_p.y;
}
bool operator^(const Square &S1,const Square &S2){//�Ƿ��ཻ
	long x=S1.centre.x -S2.centre.x;
	long y=S1.centre.y -S2.centre.y;
	x	=(x>=0)?x:-x;
	y	=(y>=0)?y:-y;
	long radius	=S1.radius	+S2.radius;
	return (x<radius)&&(y<radius);
}
bool operator^(const Circle &C1,const Circle &C2){//�ཻ����
	long x=C1.centre.x -C2.centre.x;
	long y=C1.centre.y -C2.centre.y;
	x	=(x>=0)?x:-x;
	y	=(y>=0)?y:-y;
	long radius	=C1.radius	+C2.radius;
	bool retval	=	(x<radius)&&(y<radius);
	if(retval){
		retval	=(x*x+y*y)<(radius*radius);
	}
	return	retval;
}
bool operator^(const Rect &R1,const Rect &R2){	  //�ཻ����
	long x=(R1.ltop.x	+	R1.rbtm.x)	-(R2.ltop.x	+	R2.rbtm.x);
	long y=(R1.ltop.y	+	R1.rbtm.y)	-(R2.ltop.y	+	R2.rbtm.y);
	x	=(x>=0)?x:-x;
	y	=(y>=0)?y:-y;
	long xr =(R1.rbtm.x	-	R1.ltop.x)	+(R2.rbtm.x	-	R2.ltop.x);
	long yr =(R1.rbtm.y	-	R1.ltop.y)	+(R2.rbtm.y	-	R2.ltop.y);
	return (x<xr)&&(y<yr);
}

const float g_sin[91]={0.f,0.0174524f,0.0348995f,0.052336f,0.0697565f,0.0871557f,0.104528f,0.121869f,0.139173f,0.156434f,0.173648f,0.190809f,0.207912f,0.224951f,0.241922f,0.258819f,0.275637f,0.292372f,0.309017f,0.325568f,0.34202f,0.358368f,0.374607f,0.390731f,0.406737f,0.422618f,0.438371f,0.45399f,0.469472f,0.48481f,0.5f,0.515038f,0.529919f,0.544639f,0.559193f,0.573576f,0.587785f,0.601815f,0.615662f,0.62932f,0.642788f,0.656059f,0.669131f,0.681998f,0.694658f,0.707107f,0.71934f,0.731354f,0.743145f,0.75471f,0.766044f,0.777146f,0.788011f,0.798635f,0.809017f,0.819152f,0.829038f,0.838671f,0.848048f,0.857167f,0.866025f,0.87462f,0.882948f,0.891007f,0.898794f,0.906308f,0.913545f,0.920505f,0.927184f,0.93358f,0.939693f,0.945519f,0.951057f,0.956305f,0.961262f,0.965926f,0.970296f,0.97437f,0.978148f,0.981627f,0.984808f,0.987688f,0.990268f,0.992546f,0.994522f,0.996195f,0.997564f,0.99863f,0.999391f,0.999848f,1.f};
const float g_cos[91]={1.f,0.999848f,0.999391f,0.99863f,0.997564f,0.996195f,0.994522f,0.992546f,0.990268f,0.987688f,0.984808f,0.981627f,0.978148f,0.97437f,0.970296f,0.965926f,0.961262f,0.956305f,0.951057f,0.945519f,0.939693f,0.93358f,0.927184f,0.920505f,0.913545f,0.906308f,0.898794f,0.891007f,0.882948f,0.87462f,0.866025f,0.857167f,0.848048f,0.838671f,0.829038f,0.819152f,0.809017f,0.798635f,0.788011f,0.777146f,0.766044f,0.75471f,0.743145f,0.731354f,0.71934f,0.707107f,0.694658f,0.681998f,0.669131f,0.656059f,0.642788f,0.62932f,0.615662f,0.601815f,0.587785f,0.573576f,0.559193f,0.544639f,0.529919f,0.515038f,0.5f,0.48481f,0.469472f,0.45399f,0.438371f,0.422618f,0.406737f,0.390731f,0.374607f,0.358368f,0.34202f,0.325568f,0.309017f,0.292372f,0.275637f,0.258819f,0.241922f,0.224951f,0.207912f,0.190809f,0.173648f,0.156434f,0.139173f,0.121869f,0.104528f,0.0871557f,0.0697565f,0.052336f,0.0348995f,0.0174524f,0.f};
const float g_tan[90]={0.f,0.0174551f,0.0349208f,0.0524078f,0.0699268f,0.0874887f,0.105104f,0.122785f,0.140541f,0.158384f,0.176327f,0.19438f,0.212557f,0.230868f,0.249328f,0.267949f,0.286745f,0.305731f,0.32492f,0.344328f,0.36397f,0.383864f,0.404026f,0.424475f,0.445229f,0.466308f,0.487733f,0.509525f,0.531709f,0.554309f,0.57735f,0.600861f,0.624869f,0.649408f,0.674509f,0.700208f,0.726543f,0.753554f,0.781286f,0.809784f,0.8391f,0.869287f,0.900404f,0.932515f,0.965689f,1.f,1.03553f,1.07237f,1.11061f,1.15037f,1.19175f,1.2349f,1.27994f,1.32704f,1.37638f,1.42815f,1.48256f,1.53987f,1.60033f,1.66428f,1.73205f,1.80405f,1.88073f,1.96261f,2.0503f,2.14451f,2.24604f,2.35585f,2.47509f,2.60509f,2.74748f,2.90421f,3.07768f,3.27085f,3.48741f,3.73205f,4.01078f,4.33148f,4.70463f,5.14455f,5.67128f,6.31375f,7.11537f,8.14435f,9.51436f,11.4301f,14.3007f,19.0811f,28.6363f,57.29f};

short arctan(const Point &src,const Point &dest){
	long l_dltx	=dest.x	-src.x;
	long l_dlty	=dest.y	-src.y;
	short	l_dir;
	if(l_dltx !=0){
		float	l_tan	=float(l_dlty)/l_dltx;
		l_tan	=l_tan<0?-l_tan:l_tan;
		if(l_tan >g_tan[0]){
			for(l_dir=1;l_dir<90;l_dir++){
				if((g_tan[l_dir-1]<l_tan)&&(g_tan[l_dir]>=l_tan)){
					break;
				}
			}
		}else{
			l_dir	=0;
		}
	}else{
		if(l_dlty ==0){
			return 0;
		}
		l_dir	=90;
	}
	if(l_dltx>=0 && l_dlty <0){
		l_dir =90-l_dir;
	}else if(l_dltx >=0 && l_dlty >=0){
		l_dir =90+l_dir;
	}else if(l_dltx <0 && l_dlty >=0){
		l_dir =270 -l_dir;
	}else if(l_dltx <0 && l_dlty <0){
		l_dir =270 +l_dir;
	}
	return l_dir;
}
