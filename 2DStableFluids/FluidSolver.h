#include "SparseMatrix.h"

#pragma once
class vec2
{
public:
	double x, y;
	vec2():x(0.),y(0.){};
	vec2(double a, double b):x(a),y(b){};
	vec2 operator*(double s) {return vec2(s*x,s*y);};
	vec2 operator+(vec2 & v) {return vec2(x+v.x,y+v.y);};
	vec2& operator=(vec2 & v) {x=v.x; y=v.y; return *this;};
};

class CFluidSolver
{
public:
	int		n;		// number of grid points along one side of the square domain
	int		size;	// = n * n
	double	h;		// time step
	double  viscosity; // Viscosity coefficient

	double*	density;
	vec2*	velocity;
	double* pressure;
	double* divergence;

	double* density_source;
	vec2*	velocity_source;
	vec2*	advected_velocity; //temp variable

	// Temporary arrays for velocity diffusion components
	double* vel_x;
	double* vel_y;
	double* new_vel_x;
	double* new_vel_y;
	
	CSparseMatrix laplacian;
	CSparseMatrix diffusion; // For density
	CSparseMatrix velocity_diffusion_matrix; // For velocity

public:
	void reset();
	void update();
	void updateVelocity();
	void updateDensity();
	void clean_density_source();
	void clean_velocity_source();
	void projection();
	void density_advection();
	void velocity_advection();
	void velocity_diffusion();
	void update_velocity_diffusion_matrix();

	vec2* v(int i, int j) {return velocity+i+j*n;};
	double* d(int i, int j) {return density+i+j*n;};
	double p(int i, int j) {return pressure[i+j*n];};
	void add(double* c, double* a, double* b)
	{
		for(int i = 0; i < size; i++) {
			c[i] = a[i] + b[i];
		}
	};

	void add(vec2* c, vec2* a, vec2* b)
	{
		for(int i = 0; i < size; i++) {
			c[i] = a[i] + b[i];
		}
	};

	CFluidSolver(void);
	~CFluidSolver(void);
};

