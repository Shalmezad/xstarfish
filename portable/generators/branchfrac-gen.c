/*

Copyright ©1999 Mars Saxman
All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


Branchfractal Generator

Creates an iterative fern or tree like branching fractal.
Pixel values are calculated based on distance from the nearest
stem. This is one of the more complex algorithms. It would be nice
to make it even more complicated by adding a third dimension...
but that's off in the future somewhere. It would also be nice
to add curves.

*/

#include "branchfrac-gen.h"
#include "genutils.h"
#include <math.h>
#include <stdlib.h>

#define MAX_BRANCHES 8
#define CLOSE_ENOUGH 0.0001
#define MAX_RAYS 128			//must be less than max(short)
#define ROOT_RAY 0
#define MAX_PARENTS 5

typedef struct Ray
	{
	float h,v;			//where does the ray originate?
	float angle;		//and which way does it point?
	float length;		//how long, in that way, does it go?
	int ancestors;		//how many ancestors does this ray have?
	int children;		//and how many children?
	short leaf[MAX_BRANCHES];	//which children belong to this ray?
	}
Ray;

typedef struct Tree
	{
	int branchmin, branchmax;	//how many branches do we add, minimum and maximum?
	float divgmin, divgmax;		//how far apart can the branches be?
	float scalemin, scalemax;	//by how much do we scale down each branch?
	float twistmin, twistmax;	//how far do we rotate branches from the base?
	int raycount;				//how many rays in the array are allocated?
	Ray branch[MAX_RAYS];		//all the rays in this tree
	}
Tree;

static float TangentRayDistance(float h, float v, Ray* ray);
static float ValueFromRay(float h, float v, Ray* ray, Tree* tree);
static float ValueFromBranches(float h, float v, Ray* ray, Tree* tree);
static int AllocRay(Tree* tree);
static void MakeLeaves(Ray* ray, Tree* tree);
static void MakeBranch(const Ray* ray, int whichbranch, Ray* branch, Tree* tree);

void* BranchfracInit(void)
	{
	/*
	Make a new tree.
	We will pick random branching and divergence values.
	These will be used to calculate the rest of the fractal.
	*/
	Tree* out = (Tree*)malloc(sizeof(Tree));
	if(out)
		{
		Ray* root = &out->branch[0];
		out->raycount = 1;
		//The root of the tree always starts in the centre of the image.
		root->h = root->v = 0.5;
		//Pick some random values for our root.
		root->angle = frand(pi * 2);
		root->length = 0.2;
		root->ancestors = 0;
		root->children = 0;
		out->scalemin = 0.7;//frand(0.5) + 0.5;
		out->scalemax = 0.7;
		//Pick how many branches this tree may have. Range is from 1 through 8 per node.
		//Min can never be greater than max, and max can never be less than 1.
		out->branchmax = 4;//round(frand(3.0) + 1.0);
		out->branchmin = 2;//(rand() * out->branchmax) / RAND_MAX;
		//Now pick some random divergence values - this determines how bushy the tree will be.
		//We go from eighth-pi through half-pi.
		out->divgmax = frand(pi / 2);
		out->divgmin = frand(out->divgmax - (pi / 8)) + (pi / 8);
		//That's all the parameters we need for the fractal.
		//Now go build the tree.
		MakeLeaves(root, out);
		}
	return out;
	}

static int AllocRay(Tree* tree)
	{
	//Allocate the next ray in the tree. Kinda simple.
	if(tree->raycount >= MAX_RAYS) return 0;
	return tree->raycount++;
	}

static void MakeLeaves(Ray* ray, Tree* tree)
	{
	/*
	Create a bunch of branches for this ray.
	We pick a random number of branches, then iterate through
	each one and create it separately. Each one in turn creates
	*its* own set of branches.
	*/
	int ctr;
	ray->children = irandge(tree->branchmin, tree->branchmax);
	for(ctr = 0; ctr < ray->children; ctr++)
		{
		int leafindex;
		Ray* leaf;
		leafindex = AllocRay(tree);
		ray->leaf[ctr] = leafindex;
		if(leafindex)
			{
			leaf = &tree->branch[leafindex];
			MakeBranch(ray, ctr, leaf, tree);
			}
		}
	}

void BranchfracExit(void* refcon)
	{
	/*
	Throw away our tree, if it was created successfully.
	*/
	if(refcon) free(refcon);
	}

float Branchfrac(float h, float v, void* refcon)
	{
	/*
	We have a point and an angle.
	Find the distance from (h,v) to the tangent point on the ray.
	*/
	float out = 0;
	Tree* glb = (Tree*)refcon;
	//Get the distance to the nearest branch.
	out = ValueFromRay(h, v, &glb->branch[ROOT_RAY], glb);
	//Prettify that distance so it becomes an outputable value.
	//We will eventually have several ways to do this.
	return 1 / ((out * 10.0) + 1.0);
	}

static float TangentRayDistance(float h, float v, Ray* ray)
	{
	/*
	Calculate the distance from this point to the tangent point
	on the specified ray. If we are past the end of the ray's length,
	the distance is the shorter of the distance to the tangent or the
	distance to the endpoint.
	*/
	float pointangle, distance;
	float leg;
	pointangle = atan((ray->v - v) / (ray->h - h)) + ray->angle;
	distance = fabs(cos(pointangle) * hypotf(ray->v - v, ray->h - h));
	leg = sin(pointangle) * hypotf(ray->v - v, ray->h - h);
	if(ray->h < h) leg = -leg;
	//This algorithm results in a smooth, circular glow around the ends of the ray.
	if(leg < 0.0)
		{
		distance = hypotf(distance, leg);
		}
	if(leg > ray->length)
		{
		distance = hypotf(distance, leg - ray->length);
		}
	//This algorithm, not currently in use, creates funky-cool glowing rays.
	//The smooth glow is preferred because it is more organicky.
	//if(leg < 0.0) distance += fabs(leg);
	//if(leg > ray->length) distance += fabs(leg - ray->length);
	return distance;
	}

static float ValueFromRay(float h, float v, Ray* ray, Tree* tree)
	{
	/*
	Calculate the value of the pixel, according to this ray.
	We first calculate the value according to the tangent from the ray itself.
	Then we check the ray's branches, in case they happen to be closer to the ray.
	*/
	float out, branchbest;
	out = TangentRayDistance(h, v, ray);
	//If we aren't within the "close enough" threshold, see what the branches have.
	if(out > CLOSE_ENOUGH && ray->ancestors < MAX_PARENTS && ray->children > 0)
		{
		branchbest = ValueFromBranches(h, v, ray, tree);
		if(branchbest < out) out = branchbest;
		}
	return out;
	}

static float ValueFromBranches(float h, float v, Ray* ray, Tree* tree)
	{
	//For testing purposes, make a single branch and return its value.
	float out, test;
	int ctr;
	Ray branch;
	out = 1 / 0.000000001;	//as close to 1/0 as the compiler will let me
	for(ctr = 0; ctr < ray->children; ctr++)
		{
		Ray* leaf;
		if(ray->leaf[ctr])
			{
			leaf = &tree->branch[ray->leaf[ctr]];
			test = ValueFromRay(h, v, leaf, tree);
			if(test < out) out = test;
			}
		}
	return out;
	}

static void MakeBranch(const Ray* ray, int whichbranch, Ray* branch, Tree* tree)
	{
	/*
	For testing purposes, we follow a hard-wired formula:
	The angle is 90 degrees off. The length is half. 
	*/
	//Position the origin of this branch at the end of the parent ray.
	branch->length = ray->length * tree->scalemax;// 2;
	branch->h = ray->h - sin(ray->angle) * ray->length;
	branch->v = ray->v - cos(ray->angle) * ray->length;
	//Now pick an angle for it, based on its position within the group.
	//branch->angle = ray->angle - pi / 2;
	//branch->angle += (pi / 6) * whichbranch;
	branch->angle = frand(pi);
	branch->ancestors = ray->ancestors + 1;
	if(branch->ancestors < MAX_PARENTS) MakeLeaves(branch, tree);
	}