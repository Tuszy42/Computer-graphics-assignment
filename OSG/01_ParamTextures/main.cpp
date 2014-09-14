#include <osg/Texture2D>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/PolygonMode>

#ifdef _DEBUG
	#pragma comment(lib, "osgd.lib")
	#pragma comment(lib, "osgDBd.lib")
	#pragma comment(lib, "osgViewerd.lib")
#else
	#pragma comment(lib, "osg.lib")
	#pragma comment(lib, "osgDB.lib")
	#pragma comment(lib, "osgViewer.lib")
#endif

#pragma warning(disable : 4482 )

void calc(float x, float y, 
	osg::ref_ptr<osg::Vec3Array>& vertices,
	osg::ref_ptr<osg::Vec3Array>& normals,
	osg::ref_ptr<osg::Vec2Array>& texcoords
){
	float u = 2 * osg::PI * x;
	float v = osg::PI * y;

	vertices->push_back( osg::Vec3(cos(u) * sin(v), sin(u) * sin(v), cos(v)) );
	normals->push_back( osg::Vec3(cos(u) * sin(v), sin(u) * sin(v), cos(v)) );
	//vertices->push_back( osg::Vec3(cos(u), 10*y, sin(u)) );
	//normals->push_back( osg::Vec3(cos(u), 0, sin(u)) );
	texcoords->push_back( osg::Vec2(x, y) );
}

int main( int argc, char** argv )
{
	// pozíciók
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
	
	const int N = 32;
	float delta = 1.0/N;
	for(int i=0; i<N; ++i){
		for(int j=0; j<N; ++j){
			float x = i*delta;
			float y = j*delta;
			// 1. háromszög: x,y x+delta,y y+delta,x
			calc(x+delta, y, vertices, normals, texcoords);
			calc(x, y, vertices, normals, texcoords);
			calc(x, y+delta, vertices, normals, texcoords);
			// 2. háromszög: x+delta,y x+delta,y+delta y+delta,x
			calc(x+delta, y+delta, vertices, normals, texcoords);
			calc(x+delta, y, vertices, normals, texcoords);
			calc(x, y+delta, vertices, normals, texcoords);
		}
	}
	// négyszög geometria
	osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
	quad->setUseVertexBufferObjects(true);

	// állítsuk be, hogy a VBO-ba milyen adatok kerüljenek
	quad->setVertexArray( vertices.get() );
	quad->setNormalArray( normals.get() );
	quad->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
	quad->setTexCoordArray( 0, texcoords.get() );
	// kirajzolandó primitív meghatározása
	quad->addPrimitiveSet( new osg::DrawArrays(GL_TRIANGLES, 0, 6*N*N) );

	// rakjuk be egy geode-ba a quad-ot, mint kirajzolandó elemet!
	osg::ref_ptr<osg::Geode> root = new osg::Geode;
	root->addDrawable( quad.get() );

	//osgDB::writeNodeFile(*root,"gomb.obj");

	// hozzuk létre a viewer-t és állítsuk be a gyökeret megjelenítendõ adatnak
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
	
	// a (20,20) kezdeti pozícióba hozzunk létre egy 640x480-as ablakot
    viewer.setUpViewInWindow(20, 20, 640, 480); 
    viewer.realize(); 

	// adjuk át a vezérlést a viewer-nek
    return viewer.run();
}