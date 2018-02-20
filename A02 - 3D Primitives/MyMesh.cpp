#include "MyMesh.h"
#include <vector>
#include "math.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);

	Release();
	Init();

	float numSub = 360.0f / a_nSubdivisions;

	std::vector<vector3> points;
	vector3 point;

	vector3 base(0.0f, a_fHeight * -.5, 0.0f);

	//loop through and generate all points for the base of the cone
	for (float i = 0; i < 360; i += numSub) {
	
		//each point on the base
		point = vector3((0.0f + a_fRadius * cos(i * PI / 180.0f)), a_fHeight * -.5, (0.0f + a_fRadius * sin(i * PI / 180.0f)));

		//push to array
		points.push_back(point);
	
	}
	
	//loop counterclockwise

	//draw bottom
	for (int i = points.size() - 1; i > -1; i -= 1) {

		//goes backwards since on bottom
		if ((i - 1) >= 0) {

			AddTri(points[i - 1], points[i], vector3(0.0f, a_fHeight * -.5, 0.0f));

		}
		else {

			//draw overflow panel, reset to 0.
			AddTri(points[points.size() - 1], points[i], vector3(0.0f, a_fHeight * -.5, 0.0f));

		}

	}
	//draw top/sides
	//go in order since facing upright
	for (int i = points.size() - 1; i > -1; i -= 1) {

		if ((i - 1) >= 0) {

			AddTri(points[i], points[i-1], vector3(0.0f, (a_fHeight * .5),0.0f));

		}
		else {

			//draw overflow panel, reset to 0.
			AddTri(points[i], points[points.size() - 1], vector3(0.0f, (a_fHeight * .5), 0.0f));

		}

	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	
	//get number of panels
	float numSub = 360.0f / a_nSubdivisions;

	//two arrays, one for top points and one for bottom points(essentailly a 2d vector)
	std::vector<vector3> pointsT;//top
	std::vector<vector3> pointsB;//bottom
	vector3 point;
	vector3 point2;

	vector3 top(0.0f, a_fHeight * .5, 0.0f);

	//loop through and create all points
	for (float i = 0; i < 360; i += numSub) {

		point = vector3((0.0f + a_fRadius * cos(i * PI / 180.0f)), a_fHeight * -.5, (0.0f + a_fRadius * sin(i * PI / 180.0f)));

		point2 = vector3((0.0f + a_fRadius * cos(i * PI / 180.0f)), a_fHeight * .5, (0.0f + a_fRadius * sin(i * PI / 180.0f)));

		pointsB.push_back(point);
		pointsT.push_back(point2);

	}
	//loop through and create all panels
	for (int i = pointsT.size() - 1; i > -1; i -= 1) {

		if ((i - 1) >= 0) {

			AddQuad(pointsB[i], pointsB[i - 1], pointsT[i], pointsT[i-1]);

		}
		else {
	
			//draw overflow panel, reset to 0.
			AddQuad(pointsB[i], pointsB[pointsB.size()-1], pointsT[i], pointsT[pointsT.size()-1]);

		}

	}
	//loop through and cover up all tops and bottoms
	for (int i = pointsT.size() - 1; i > -1; i -= 1) {

		if ((i - 1) >= 0) {

			AddTri(pointsT[i], pointsT[i-1], top);

			AddTri(pointsB[i - 1], pointsB[i], vector3(0.0f, a_fHeight * -.5, 0.0f));

		}
		else {

			//draw overflow panel, reset to 0.
			AddTri(pointsT[i], pointsT[pointsT.size() - 1], top);

			AddTri(pointsB[pointsB.size() - 1], pointsB[i], vector3(0.0f, a_fHeight * -.5, 0.0f));

		}

	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);

	float numSub = 360.0f / a_nSubdivisions;

	//4 arrays, used for tops, bottoms, and the inner and outer vaules.
	std::vector<vector3> pointsTI;
	std::vector<vector3> pointsBI;
	std::vector<vector3> pointsTO;
	std::vector<vector3> pointsBO;
	vector3 point;
	vector3 point2;

	vector3 top(0.0f, a_fHeight * .5, 0.0f);

	//outer circle
	for (float i = 0; i < 360; i += numSub) {

		//generate bottom values
		point = vector3((0.0f + a_fOuterRadius * cos(i * PI / 180.0f)), a_fHeight * -.5, (0.0f + a_fOuterRadius * sin(i * PI / 180.0f)));

		//generate top values
		point2 = vector3((0.0f + a_fOuterRadius * cos(i * PI / 180.0f)), a_fHeight * .5, (0.0f + a_fOuterRadius * sin(i * PI / 180.0f)));

		pointsBO.push_back(point);
		pointsTO.push_back(point2);

	}
	//inner circle
	for (float i = 0; i < 360; i += numSub) {

		//generate bottom values
		point = vector3((0.0f + a_fInnerRadius * cos(i * PI / 180.0f)), a_fHeight * -.5, (0.0f + a_fInnerRadius * sin(i * PI / 180.0f)));

		//generate top values
		point2 = vector3((0.0f + a_fInnerRadius * cos(i * PI / 180.0f)), a_fHeight * .5, (0.0f + a_fInnerRadius * sin(i * PI / 180.0f)));

		pointsBI.push_back(point);
		pointsTI.push_back(point2);

	}
	//loop thorugh and draw all panels
	for (int i = pointsTI.size() - 1; i > -1; i -= 1) {

		if ((i - 1) >= 0) {

			//link tops to bottoms
			AddQuad(pointsBO[i], pointsBO[i - 1], pointsTO[i], pointsTO[i - 1]);
			AddQuad(pointsTI[i], pointsTI[i - 1], pointsBI[i], pointsBI[i - 1]);

		}
		else {

			//draw overflow panel, reset to 0.
			AddQuad(pointsBO[i], pointsBO[pointsBO.size() - 1], pointsTO[i], pointsTO[pointsTO.size() - 1]);
			AddQuad(pointsTI[i], pointsTI[pointsTI.size() - 1], pointsBI[i], pointsBI[pointsBO.size() - 1]);

		}

	}
	//loop through and draw all tops
	for (int i = pointsTI.size() - 1; i > -1; i -= 1) {

		if ((i - 1) >= 0) {

			//link all 4 corresponding points on one side || top or bottom
			AddQuad(pointsBI[i], pointsBI[i - 1], pointsBO[i], pointsBO[i - 1]);
			AddQuad(pointsTO[i], pointsTO[i - 1], pointsTI[i], pointsTI[i - 1]);

		}
		else {

			//draw overflow panel, reset to 0.
			AddQuad(pointsBI[i], pointsBI[pointsBI.size() - 1], pointsBO[i], pointsBO[pointsBO.size() - 1]);
			AddQuad(pointsTO[i], pointsTO[pointsTO.size() - 1], pointsTI[i], pointsTI[pointsTI.size() - 1]);

		}

	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();
	
	float numSubA = 180.0f / a_nSubdivisions;//Loop around Z
	float numSubB = 360.0f / a_nSubdivisions;//Loop around Y
	//float width = a_fRadius / (a_nSubdivisions/2);

	vector3 top(0.0f, a_fRadius * 2, 0.0f);

	std::vector<std::vector<vector3> > points; //all points in the circle
	//std::vector<vector3> pointsB;
	vector3 point;
	//vector3 point2;
	
	///loop through and create all points
	for (float i = -90; i <= 90; i += numSubA) {//loop around Z axis

		std::vector<vector3> temp;//temp array to store values 

		for (float j = 0; j <= 360; j += numSubB) {//loop around Y axis

			//generate point
			point = vector3(
				a_fRadius * cos(i * PI / 180.0f) *  cos(j * PI / 180.0f),
				a_fRadius * cos(i * PI / 180.0f) *  sin(j * PI / 180.0f),
				a_fRadius * sin(i * PI / 180.0f)
			);

			temp.push_back(point);//add to temp array

		}

		points.push_back(temp);//add to main array

	}

	//int columns = points.size() / points.size();

	// && i != points.size() - 1 && j != points.size() - 1

	///looop thorugh the array
	for (int i = points.size() - 1; i > -1; i -= 1) {//row
		for (int j = points.size() - 1; j > -1; j -= 1) {//column
			if ((i - 1) >= 0 && (j - 1) >= 0) {

				//draw panel
				AddQuad(points[i][j], points[i][j-1], points[i-1][j], points[i-1][j-1]);

				//AddQuad(pointsB[i], pointsB[i - 1], pointsT[i], pointsT[i - 1]);
				//AddQuad(pointsT[i], pointsT[i - 1], pointsB[i], pointsB[i - 1]);

			}
			else {

				//draw overflow panel, reset to 0.
				AddQuad(points[i][j], points[i][0], points[0][j], points[0][0]);
				//AddQuad(pointsT[i], pointsT[pointsT.size() - 1], pointsB[i], pointsB[pointsB.size() - 1]);

			}

		}
	}
	
	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}