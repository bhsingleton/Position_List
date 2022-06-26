#ifndef _PositionListNode
#define _PositionListNode
//
// File: PositionListNode.h
//
// Dependency Graph Node: positionList
//
// Author: Benjamin H. Singleton
//

#include <utility>
#include <map>
#include <vector>

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MDistance.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MMatrix.h>
#include <maya/MFloatArray.h>
#include <maya/MString.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MGlobal.h>

 
struct PositionListItem
{

	MString	name = "";
	float weight = 1.0;
	bool absolute = false;
	MVector position = MVector::zero;

};


class PositionList : public MPxNode
{

public:

						PositionList();
	virtual				~PositionList();

	virtual MStatus		compute(const MPlug& plug, MDataBlock& data);
	static  void*		creator();
	static  MStatus		initialize();
	
	static	MVector		average(const std::vector<PositionListItem>& items);
	static	void		normalize(std::vector<PositionListItem>& items);

	static	MMatrix		createPositionMatrix(const double x, const double y, const double z);
	static	MMatrix		createPositionMatrix(const MVector& position);
	
public:

	static	MObject		active;
	static	MObject		normalizeWeights;
	static	MObject		list;
	static	MObject		name;
	static	MObject		weight;
	static	MObject		absolute;
	static	MObject		positionX;
	static	MObject		positionY;
	static	MObject		positionZ;
	static	MObject		position;
	
	static	MObject		value;
	static	MObject		valueX;
	static	MObject		valueY;
	static	MObject		valueZ;
	static	MObject		matrix;
	
	static	MTypeId		id;
	static	MString		listCategory;
	static	MString		positionCategory;
	static	MString		outputCategory;

};

#endif