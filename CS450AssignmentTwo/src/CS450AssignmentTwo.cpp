// Simple 2D OpenGL Program

//Includes vec, mat, and other include files as well as macro defs
#define GL3_PROTOTYPES

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#include "Angel.h"
#include <stdio.h>
#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif

// My includes
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
// globals
const std::string DATA_DIRECTORY_PATH = "./Data/";
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location


point4 points[NumVertices];
vec4   normals[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};


//----------------------------------------------------------------------------

/* ObjObject serves to conveniently package the data found in an obj file. */
class ObjObject
{
public:
	// constructors & destructors
	ObjObject();
	ObjObject(string filename);
	ObjObject(string filename, int vertex_element_size, int texture_coord_element_size, int param_space_vertex_element_size);
	~ObjObject();

	// methods
	int load_from_file(string filename);
	int add_vertex(GLfloat vertex_x, GLfloat vertex_y, GLfloat vertex_z, GLfloat vertex_w = NULL);
	int add_texture_coord(GLfloat texture_coord_u, GLfloat texture_coord_v, GLfloat texture_coord_w = NULL);
	int add_normal(GLfloat normal_x, GLfloat normal_y, GLfloat normal_z);
	int add_param_vertex(GLfloat vertex_u, GLfloat vertex_v = NULL, GLfloat vertex_w = NULL);

	// data
	enum face_data_format {
		VERTEX, 
		VETEX_TEXTURE, 
		VERTEX_TEXTURE_NORMAL, 
		VERTEX_NORMAL
	};
	face_data_format faces_format;
	vector<GLfloat> vertices;
	vector<GLfloat> param_space_vertices;
	vector<GLfloat> texture_coords;
	vector<GLfloat> normals;
	vector<GLint> faces;

	// file characteristics / metadata
	string filename;
	bool is_loaded;
	bool bad_file;

	int size_of_vertex_element;
	int size_of_texture_coord_element;
	int size_of_param_space_vertex_element;
	int size_of_face_element;
};

vector<string> inline StringSplit(const string &source, const char *delimiter = " ", bool keepEmpty = false)
{
    std::vector<std::string> results;

    size_t prev = 0;
    size_t next = 0;

    while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
    {
        if (keepEmpty || (next - prev != 0))
        {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next + 1;
    }

    if (prev < source.size())
    {
        results.push_back(source.substr(prev));
    }

    return results;
}


ObjObject::~ObjObject()
{
	size_of_vertex_element = NULL;
	size_of_texture_coord_element = NULL;
	size_of_param_space_vertex_element = NULL;
	size_of_face_element = NULL;
	bad_file = NULL;
}
ObjObject::ObjObject()
{
	size_of_vertex_element = 3;
	size_of_texture_coord_element = 2;
	size_of_param_space_vertex_element = 1;
	size_of_face_element = 1;
}

ObjObject::ObjObject(string in_filename)
{
	size_of_vertex_element = 3;
	size_of_texture_coord_element = 2;
	size_of_param_space_vertex_element = 1;
	size_of_face_element = 1;
	filename = in_filename;
	
	this->load_from_file(filename);
}

ObjObject::ObjObject(string in_filename, int vertex_element_size, int texture_coord_element_size, int param_space_vertex_element_size)
{
	size_of_vertex_element = vertex_element_size;
	size_of_texture_coord_element = texture_coord_element_size;
	size_of_param_space_vertex_element = param_space_vertex_element_size;
	filename = in_filename;

	this->load_from_file(filename);
}

int ObjObject::add_vertex(GLfloat vertex_x, GLfloat vertex_y, GLfloat vertex_z, GLfloat vertex_w)
{
	this->vertices.push_back(vertex_x);
	this->vertices.push_back(vertex_y);
	this->vertices.push_back(vertex_z);

	if(this->size_of_vertex_element == 4)
		this->vertices.push_back(vertex_w);
	return this->vertices.size();
}

int ObjObject::add_texture_coord(GLfloat texture_coord_u, GLfloat texture_coord_v, GLfloat texture_coord_w)
{
	this->texture_coords.push_back(texture_coord_u);
	this->texture_coords.push_back(texture_coord_v);
	this->texture_coords.push_back(texture_coord_w);

	if(this->size_of_texture_coord_element == 4)
		this->texture_coords.push_back(texture_coord_w);
	return this->texture_coords.size();
}

int ObjObject::add_normal(GLfloat normal_x, GLfloat normal_y, GLfloat normal_z)
{
	this->normals.push_back(normal_x);
	this->normals.push_back(normal_y);
	this->normals.push_back(normal_z);
	return this->normals.size();
}

int ObjObject::add_param_vertex(GLfloat vertex_u, GLfloat vertex_v, GLfloat vertex_w)
{
	int element_size = this->size_of_param_space_vertex_element;

	this->param_space_vertices.push_back(vertex_u);

	if(element_size >= 2)
		this->param_space_vertices.push_back(vertex_v);

	if(element_size == 3)
		this->param_space_vertices.push_back(vertex_w);

	return this->param_space_vertices.size();
}

int ObjObject::load_from_file(string in_filename)
{
	filename = in_filename;
	ifstream in_file(in_filename);
	string line;
	GLfloat comp0, comp1, comp2, comp3; // generic names for components of various points/vectors in the obj file
	int vertex_size;
	int status = -1;
	if(in_file.is_open())
	{
		while(!in_file.eof())
		{
			getline(in_file, line);
			auto tokens = StringSplit(line);
			for(auto t : tokens) {
				if(tokens[0] == "v") 
				{
					vertex_size = tokens.size() - 1;
					size_of_vertex_element = vertex_size;
					comp0 = atof(tokens[1].c_str());
					comp1 = atof(tokens[2].c_str());
					comp2 = atof(tokens[3].c_str());

					comp3 = NULL;
					if(size_of_vertex_element == 4)
						comp3 = atof(tokens[4].c_str());

					this->add_vertex(comp0, comp1, comp2, comp3);
					break;
				}  else if(tokens[0] == "vt") {
					size_of_texture_coord_element = (tokens.size() - 1);
					comp0 = atof(tokens[1].c_str());
					comp1 = atof(tokens[2].c_str());

					comp2 = NULL;
					if(size_of_texture_coord_element == 3)
						comp3 = atof(tokens[1].c_str());

					this->add_texture_coord(comp0, comp1, comp2);
					break;
				} else if(tokens[0] == "vn") {
					comp0 = atof(tokens[1].c_str());
					comp1 = atof(tokens[2].c_str());
					comp2 = atof(tokens[3].c_str());

					this->add_normal(comp0, comp1, comp2);
					break;
				} else if(tokens[0] == "vp") {
					size_of_param_space_vertex_element = (tokens.size() - 1);
					comp0 = atof(tokens[1].c_str());

					comp1 = NULL;
					comp2 = NULL;
					if(size_of_param_space_vertex_element >= 2)
						comp1 = atof(tokens[2].c_str());

					if(size_of_param_space_vertex_element == 3)
						comp2 = atof(tokens[3].c_str());

					this->add_param_vertex(comp0, comp1, comp2);
					break;
				} else if(tokens[0] == "f") {
					vector<string> f1 = StringSplit(tokens[1], "/");
					vector<string> f2 = StringSplit(tokens[2], "/");
					vector<string> f3 = StringSplit(tokens[3], "/");
					// TODO: Not sure how faces should be loaded into vectors
					size_of_face_element = (f1.size());
					break;
				} else if(tokens[0] == "#") {
					// This is a comment line
					break;
				} else {
					// TODO: This catches the first three lines of obj file which contains file validation data. Need to handle these three lines instead of falling through here.
					cerr << "Inconceivable!" << endl;
					break;
				}
			}
		}
		cout << line << endl;
		bad_file = false;
		status = 0;
		in_file.close();
	} else {
		cerr << "Unable to open file: '" << filename << "'" << endl;
		bad_file = true;
		return status;
	}
	return status;
}
// END: ObjObject implementation

int load_scene_by_file(string filename, vector<string>& obj_filename_list)
{
	ifstream input_scene_file;
	string line;
	string filepath;
	int status = -1;

	filepath = DATA_DIRECTORY_PATH + filename;

	input_scene_file.open(filepath);
	if(input_scene_file.is_open())
	{
		getline(input_scene_file, line);
		cout << "Dimension(s) of file: '" << line << "'" << endl;
		while(!input_scene_file.eof())
		{
			getline(input_scene_file, line);
			obj_filename_list.push_back(line);
			cout << line << endl;
		}
		status = 0;
		input_scene_file.close();
	} else {
		status = -1;
	}
	return status;
}
// quad generates two triangles for each face and assigns colors
//    to the vertices.  Notice we keep the relative ordering when constructing the tris
int Index = 0;
void
quad( int a, int b, int c, int d )
{


  vec4 u = vertices[b] - vertices[a];
  vec4 v = vertices[c] - vertices[b];

  vec4 normal = normalize( cross(u, v) );
  normal[3] = 0.0;

  normals[Index] = normal; points[Index] = vertices[a]; Index++;
  normals[Index] = normal; points[Index] = vertices[b]; Index++;
  normals[Index] = normal; points[Index] = vertices[c]; Index++;
  normals[Index] = normal; points[Index] = vertices[a]; Index++;
  normals[Index] = normal; points[Index] = vertices[c]; Index++;
  normals[Index] = normal; points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
  quad( 4, 5, 6, 7 );
  quad( 5, 4, 0, 1 );
  quad( 1, 0, 3, 2 );
  quad( 2, 3, 7, 6 );
  quad( 3, 0, 4, 7 );
  quad( 6, 5, 1, 2 );
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
    colorcube();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals),
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "./src/vshader.glsl", "./src/fshader.glsl" );
    glUseProgram( program );

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );


    // Initialize shader lighting parameters
    // RAM: No need to change these...we'll learn about the details when we
    // cover Illumination and Shading
    point4 light_position( 1.5, 0.5, 2.0, 1.0 );
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

    color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
    color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
    color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
    float  material_shininess = 100.0;

    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;

    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		  1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		  1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		  1, specular_product );

    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
		  1, light_position );

    glUniform1f( glGetUniformLocation(program, "Shininess"),
		 material_shininess );


    model_view = glGetUniformLocation( program, "ModelView" );
    projection = glGetUniformLocation( program, "Projection" );



    mat4 p = Perspective(45, 1.0, 0.1, 10.0);
    point4  eye( 1.0, 1.0, 2.0, 1.0);
    point4  at( 0.0, 0.0, 0.0, 1.0 );
    vec4    up( 0.0, 1.0, 0.0, 0.0 );


    mat4  mv = LookAt( eye, at, up );
    //vec4 v = vec4(0.0, 0.0, 1.0, 1.0);

    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );


    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------



/*
 *  simple.c
 *  This program draws a red rectangle on a white background.
 *
 * Still missing the machinery to move to 3D
 */

/* glut.h includes gl.h and glu.h*/

int main(int argc, char** argv)
{
	string data_filename = "NO_DATA_FILE";
	string application_info = "CS450AssignmentTwo: ";
	string *window_title = new string;
	GLfloat eye_position[] = { 0., 0., 1., 1.};
	GLfloat at_position[] = { 0., 0., 0., 1. };
	GLfloat up_vector[] = { 0., 1., 0., 0. };

	if(argc != 11) {
		cerr << "USAGE: Expected 10 arguments but found '" << (argc - 1) << "'" << endl;
		cerr << "CS450AssignmentTwo SCENE_FILENAME FROM_X FROM_Y FROM_Z AT_X AT_Y AT_Z UP_X UP_Z UP_Y" << endl;
		cerr << "SCENE_FILENAME: A .scn filename existing in the ./CS450AssignmentTwo/Data/ directory." << endl;
		cerr << "FROM_X, FROM_Y, FROM_Z*: Floats passed to the LookAt function representing the point in the scene of the eye." << endl;
		cerr << "AT_X, AT_Y, AT_Z*: Floats passed to the LookAt function representing the point in the scene where the eye is looking." << endl;
		cerr << "UP_X, UP_Y, UP_Z*: Floats passed to the LookAt function representing the vector that describes the up direction within scene for the eye." << endl;
		cerr << "*These points and vectors will be converted to a homogenous coordinate system." << endl;
		return -1;
	}
	data_filename = argv[1];

	eye_position[0] = atof(argv[2]);
	eye_position[1] = atof(argv[3]);
	eye_position[2] = atof(argv[4]);

	at_position[0] = atof(argv[5]);
	at_position[1] = atof(argv[6]);
	at_position[2] = atof(argv[7]);

	up_vector[0] = atof(argv[8]);
	up_vector[1] = atof(argv[9]);
	up_vector[2] = atof(argv[10]);

	cout << "Loading scene file: '" << data_filename.c_str() << "'" << endl;
	cout << "Eye position: {" << eye_position[0] << ", " << eye_position[1] << ", " << eye_position[2] << "}" << endl;
	cout << "At position: {" << at_position[0] << ", " << at_position[1] << ", " << at_position[2] << "}" << endl;
	cout << "Up vector: {" << up_vector[0] << ", " << up_vector[1] << ", " << up_vector[2] << "}" << endl;
	
	vector<string> obj_filenames;
	int scene_load_status = load_scene_by_file(data_filename, obj_filenames);
    if(scene_load_status == -1)
	{
		cerr << "Unable to load file: '" << data_filename << "'" << endl;
		return -1;
	}
	vector<ObjObject> obj_object_data;
	for(auto filename : obj_filenames)
	{
		ObjObject new_obj_object(DATA_DIRECTORY_PATH + filename);
		if(new_obj_object.bad_file)
		{
			cerr << "Unable to load obj files." << endl;
			return -1;
		}
	}
	glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
#endif
	window_title->append(application_info);
	window_title->append(data_filename);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(500, 300);
    glutCreateWindow(window_title->c_str());
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    init();

    //NOTE:  callbacks must go after window is created!!!
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();

    return(0);
}
