// Text-based renderer for tiny_bvh.h

#define TINYBVH_IMPLEMENTATION
#include "tiny_bvh.h"
#ifdef _MSC_VER
#include "stdlib.h"		// for rand
#include "stdio.h"		// for printf
#else
#include <cstdlib>
#include <cstdio>
#endif

using namespace tinybvh;

#define SPHERE_COUNT	259

struct TriVertex {
	TriVertex() = default;
	TriVertex( bvhvec3 v ) : x( v.x ), y( v.y ), z( v.z ), dummy( 0 ) {}
	float x, y, z, dummy;
};
TriVertex triangles[SPHERE_COUNT * 6 * 98 * 3];
int verts = 0, spheres = 0;
// ASCII shading: https://stackoverflow.com/a/74186686
char level[92] = "`.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";

void create_sphere( float x, float y, float z, float s )
{
#define P(F,a,b,c) p[i+F*64]={(float)a ,(float)b,(float)c}
	bvhvec3 p[384], pos( x, y, z ), ofs( 3.5 );
	for (int i = 0, u = 0; u < 8; u++) for (int v = 0; v < 8; v++, i++)
		P( 0, u, v, 0 ), P( 1, u, 0, v ), P( 2, 0, u, v ),
		P( 3, u, v, 3 ), P( 4, u, 3, v ), P( 5, 3, u, v );
	for (int i = 0; i < 384; i++) p[i] = normalize( p[i] - ofs ) * s + pos;
	for (int i = 0, side = 0; side < 6; side++, i += 8 )
		for (int u = 0; u < 7; u++, i++) for (int v = 0; v < 7; v++, i++)
			triangles[verts++] = p[i], triangles[verts++] = p[i + 1],
			triangles[verts++] = p[i + 8], triangles[verts++] = p[i + 1],
			triangles[verts++] = p[i + 9], triangles[verts++] = p[i + 8];
}

void sphere_flake( float x, float y, float z, float s, int d = 0 )
{
	spheres++;
	create_sphere( x, y, z, s * 0.5f );
	if (d < 3) sphere_flake( x + s * 0.75f, y, z, s * 0.5f, d + 1 );
	if (d < 3) sphere_flake( x - s * 0.75f, y, z, s * 0.5f, d + 1 );
	if (d < 3) sphere_flake( x, y + s * 0.75f, z, s * 0.5f, d + 1 );
	if (d < 3) sphere_flake( x, x - s * 0.75f, z, s * 0.5f, d + 1 );
	if (d < 3) sphere_flake( x, y, z + s * 0.75f, s * 0.5f, d + 1 );
	if (d < 3) sphere_flake( x, y, z - s * 0.75f, s * 0.5f, d + 1 );
}

int main()
{
	// generate a sphere flake scene
	sphere_flake( 0, 0, 0, 3 );

	// build a BVH over the scene
	BVH bvh;
	bvh.Build( (bvhvec4*)triangles, verts / 3 );

	// trace 50x80 primary rays
	bvhvec3 eye( -3.5f, -1.5f, -6 ), view = normalize( bvhvec3( 3, 1.5f, 5 ) );
	bvhvec3 right = normalize( cross( bvhvec3( 0, 1, 0 ), view ) );
	bvhvec3 up = 0.8f * cross( view, right ), C = eye + 2 * view;
	bvhvec3 p1 = C - right + up, p2 = C + right + up, p3 = C - right - up;
	char line[122];
	float sum;
	for( int s, x, y = 0; y < 200; y += 4 ) 
	{
		for( x = 0; x < 480; x += 4 )
		{
			for( sum = 0, s = 0; s < 16; s++ )
			{
				float u = (float)(x + (s & 3)) / 480.0f;
				float v = (float)(y + (s >> 2)) / 200.0f;
				bvhvec3 P = p1 + u * (p2 - p1) + v * (p3 - p1);
				Ray ray( eye, normalize( P - eye ) );
				bvh.Intersect( ray );
				sum += ray.hit.t > 999 ? 0 : ray.hit.t;
			}
			float t = (sum / 16 - 2.3f) / (6.12f - 2.3f);
			int color = (int)(90.0f / (t + 1));
			line[x >> 2] = level[90 - tinybvh_clamp( color, 0, 90 )];
		}
		line[120] = '\n', line[121] = 0, printf( "%s", line );
	}
	// all done.
	printf( "\nscene: %i spheres, %i triangles.\n", spheres, verts / 3 );
	return 0;
}
