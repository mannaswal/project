#include "StdAfx.h"
#include "FluidSolver.h"

//Loosely following Jos Stam's Stable Fluids

CFluidSolver::CFluidSolver(void):
n(60), size(n*n), h(0.1), laplacian(size,size), diffusion(size,size)
{
	//default size is set to 60^2
	velocity = new vec2[size];
	velocity_source = new vec2[size];
	advected_velocity = new vec2[size];
	density = new double[size];
	density_source = new double[size];
	pressure = new double[size];
	divergence = new double[size];

	double diffusion_coef = 0.3*h;
	//Set up the Laplacian matrix and diffusion matrix
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			int index = i + j*n;
			if (i>0 && i<n-1 && j>0 && j<n-1) {
				if (i-1>0) {
					laplacian.set1Value(index, index-1, -1.0);
				}
				if (i+1<n-1) {
					laplacian.set1Value(index, index+1, -1.0);
				}
				if (j-1>0) {
					laplacian.set1Value(index, index-n, -1.0);
				}
				if (j+1<n-1) {
					laplacian.set1Value(index, index+n, -1.0);
				}
				laplacian.set1Value(index, index, 4.);
			} else {
				laplacian.set1Value(index, index, 1.);
			}
			int count = 0;
			if (i-1>0) {
				diffusion.set1Value(index, index-1, -1.0*diffusion_coef);
				count++;
			}
			if (i+1<n-1) {
				diffusion.set1Value(index, index+1, -1.0*diffusion_coef);
				count++;
			}
			if (j-1>0) {
				diffusion.set1Value(index, index-n, -1.0*diffusion_coef);
				count++;
			}
			if (j+1<n-1) {
				diffusion.set1Value(index, index+n, -1.0*diffusion_coef);
				count++;
			}
			diffusion.set1Value(index, index, 1.+count*diffusion_coef);
		}
	}

	reset();
}

void CFluidSolver::reset()
{
	for (int i = 0; i < size; i++) {
		density[i] = 0.;
		density_source[i] = 0.;
		velocity[i] = vec2(2.,3.);
		divergence[i] = 0.;
		pressure[i] = 0.;
		velocity_source[i] = vec2(0.,0.);
	}

}

CFluidSolver::~CFluidSolver(void)
{
	delete[] velocity;
	delete[] density;
	delete[] pressure;
	delete[] divergence;

	delete[] density_source;
	delete[] velocity_source;
	delete[] advected_velocity;
}

void CFluidSolver::update()
{
	updateDensity();
	//updateVelocity();
}

void CFluidSolver::updateDensity()
{
	add(density, density, density_source); // density += density_source;

	//Diffusion process
	diffusion.solve(density_source, density, 1e-8, 30); // Diffusion_matrix density_new = density_old

	density_advection();
	clean_density_source();
}

void CFluidSolver::updateVelocity()
{
	velocity_advection();
	add(velocity,advected_velocity,velocity_source);

	//projection(); 
	clean_velocity_source();
}

void CFluidSolver::projection()
{
	//set boundary condition
	for (int i=0; i< n; i++) {
		*v(0,i)=vec2(0.,0.);
		*v(n-1,i)=vec2(0.,0.);
		*v(i, 0)=vec2(0.,0.);
		*v(i, n-1)=vec2(0.,0.);
	}

	//compute divergence
	for (int i = 1; i < n-1; i++)
	{
		for (int j = 1; j < n-1; j++)
		{
			divergence[i+n*j] = 0.5*(v(i+1,j)->x-v(i-1,j)->x
				+ v(i, j+1)->y - v(i, j-1)->y);
		}
	}

	//get pressure by solving (Laplacian pressure = divergence)
	laplacian.solve(pressure, divergence, 1e-8, 10);

	//update velocity by (velocity -= gradient of pressure)
	for (int i = 1; i < n-1; i++)
	{
		for (int j = 1; j < n-1; j++)
		{
			v(i, j)->x += 0.5 * (p(i+1, j) - p(i-1, j));
			v(i, j)->y += 0.5 * (p(i, j+1) - p(i, j-1));
			*v(i,j) = *v(i,j)*1.;
		}
	}

}

void CFluidSolver::clean_density_source()
{
	for (int i=0; i < size; i++) {
		density_source[i] = 0.;
	}
}

void CFluidSolver::clean_velocity_source()
{
	for (int i=0; i < size; i++) {
		velocity_source[i] = vec2(0.,0.);
	}
}

void CFluidSolver::density_advection()
{
	for (int i=0; i<size; i++)
		density[i]=density_source[i];
	return;

	//set boundary condition
	for (int i=0; i< n; i++) {
		*d(0,i)= 0;
		*d(n-1,i)=0;
		*d(i, 0)=0;
		*d(i, n-1)=0;
	}

	vec2 backtraced_position;
	//Density advection
	for (int i = 1; i < n-1; i++)
		for (int j=1; j < n-1; j++) {
			//go backwards following the velocity field
			backtraced_position = vec2(i,j);
			backtraced_position = backtraced_position + velocity[i+n*j]*(-h); 
			if (backtraced_position.x < 0.5)
				backtraced_position.x = 0.5;
			if (backtraced_position.x > n-1.5)
				backtraced_position.x = n-1.5;
			if (backtraced_position.y < 0.5)
				backtraced_position.y = 0.5;
			if (backtraced_position.y > n-1.5)
				backtraced_position.y = n-1.5;

			//bilinear interpolation
			int i0 = (int) backtraced_position.x;
			int j0 = (int) backtraced_position.y;

			double s = backtraced_position.x - i0;
			double t = backtraced_position.y - j0;
			density[i+j*n] = (1-s)*(1-t)* density_source[i0+j0*n] + (1-s)*t* density_source[i0+(j0+1)*n] + s*(1-t)* density_source[i0+1+j0*n] + s*t* density_source[i0+1+(j0+1)*n];
		}
}

void CFluidSolver::velocity_advection()
{
	for (int i = 1; i < n-1; i++)
		for (int j=1; j < n-1; j++) {
			advected_velocity[i+j*n] = velocity[i+j*n];
		}
}