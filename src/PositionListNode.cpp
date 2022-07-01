//
// File: PositionListNode.cpp
//
// Dependency Graph Node: positionList
//
// Author: Benjamin H. Singleton
//

#include "PositionListNode.h"

MObject		PositionList::active;
MObject		PositionList::normalizeWeights;
MObject		PositionList::list;
MObject		PositionList::name;
MObject		PositionList::weight;
MObject		PositionList::absolute;
MObject		PositionList::translate;
MObject		PositionList::translateX;
MObject		PositionList::translateY;
MObject		PositionList::translateZ;

MObject		PositionList::output;
MObject		PositionList::outputX;
MObject		PositionList::outputY;
MObject		PositionList::outputZ;
MObject		PositionList::matrix;
MObject		PositionList::inverseMatrix;

MTypeId		PositionList::id(0x0013b1c5);
MString		PositionList::listCategory("List");
MString		PositionList::translateCategory("Translate");
MString		PositionList::outputCategory("Output");


PositionList::PositionList() {}
PositionList::~PositionList() {}


MStatus PositionList::compute(const MPlug& plug, MDataBlock& data) 
/**
This method should be overridden in user defined nodes.
Recompute the given output based on the nodes inputs.
The plug represents the data value that needs to be recomputed, and the data block holds the storage for all of the node's attributes.
The MDataBlock will provide smart handles for reading and writing this node's attribute values.
Only these values should be used when performing computations!

@param plug: Plug representing the attribute that needs to be recomputed.
@param data: Data block containing storage for the node's attributes.
@return: Return status.
*/
{

	MStatus status;

	// Evaluate requested plug
	//
	MObject attribute = plug.attribute(&status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnAttribute fnAttribute(attribute, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (fnAttribute.hasCategory(PositionList::outputCategory))
	{
		
		// Get input data handles
		//
		MDataHandle activeHandle = data.inputValue(PositionList::active, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MDataHandle normalizeWeightsHandle = data.inputValue(PositionList::normalizeWeights, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MArrayDataHandle listHandle = data.inputArrayValue(PositionList::list, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		// Get values from handles
		//
		short active = activeHandle.asShort();
		bool normalizeWeights = normalizeWeightsHandle.asBool();
		
		// Collect position entries
		//
		unsigned int listCount = listHandle.elementCount();
		std::vector<PositionListItem> items = std::vector<PositionListItem>(listCount);
		
		MDataHandle elementHandle, nameHandle, weightHandle, absoluteHandle, translateHandle, translateXHandle, translateYHandle, translateZHandle;
		MString name;
		float weight;
		bool absolute;
		double translateX, translateY, translateZ;
		
		for (unsigned int i = 0; i < listCount; i++)
		{
			
			// Jump to array element
			//
			status = listHandle.jumpToElement(i);
			CHECK_MSTATUS_AND_RETURN_IT(status)

			elementHandle = listHandle.inputValue(&status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			// Get element data handles
			//
			nameHandle = elementHandle.child(PositionList::name);
			weightHandle = elementHandle.child(PositionList::weight);
			absoluteHandle = elementHandle.child(PositionList::absolute);
			translateHandle = elementHandle.child(PositionList::translate);
			translateXHandle = translateHandle.child(PositionList::translateX);
			translateYHandle = translateHandle.child(PositionList::translateY);
			translateZHandle = translateHandle.child(PositionList::translateZ);
			
			// Get values from handles
			//
			name = nameHandle.asString();
			weight = weightHandle.asFloat();
			absolute = absoluteHandle.asBool();
			translateX = translateXHandle.asDistance().asCentimeters();
			translateY = translateYHandle.asDistance().asCentimeters();
			translateZ = translateZHandle.asDistance().asCentimeters();
			
			// Assign value to arrays
			//
			items[i] = PositionListItem{ name, weight, absolute, MVector(translateX, translateY, translateZ) };
			
		}
		
		// Check if weights should be normalized
		//
		if (normalizeWeights)
		{

			PositionList::normalize(items);

		}

		// Calculate weighted average
		//
		MVector translation = PositionList::average(items);
		MMatrix matrix = PositionList::createTranslationMatrix(translation);
		
		// Get output data handles
		//
		MDataHandle outputXHandle = data.outputValue(PositionList::outputX, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MDataHandle outputYHandle = data.outputValue(PositionList::outputY, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MDataHandle outputZHandle = data.outputValue(PositionList::outputZ, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MDataHandle matrixHandle = data.outputValue(PositionList::matrix, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		MDataHandle inverseMatrixHandle = data.outputValue(PositionList::inverseMatrix, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		// Update output data handles
		//
		outputXHandle.setMDistance(MDistance(translation.x, MDistance::kCentimeters));
		outputXHandle.setClean();
		
		outputYHandle.setMDistance(MDistance(translation.y, MDistance::kCentimeters));
		outputYHandle.setClean();
		
		outputZHandle.setMDistance(MDistance(translation.z, MDistance::kCentimeters));
		outputZHandle.setClean();

		matrixHandle.setMMatrix(matrix);
		matrixHandle.setClean();

		inverseMatrixHandle.setMMatrix(matrix.inverse());
		inverseMatrixHandle.setClean();

		// Mark plug as clean
		//
		status = data.setClean(plug);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		return MS::kSuccess;

	}
	else
	{

		return MS::kUnknownParameter;

	}

};


template<class N> N lerp(const N& start, const N& end, const double weight)
/**
Linearly interpolates the two given numbers using the supplied weight.

@param start: The start number.
@param end: The end number.
@param weight: The amount to blend.
@return: The interpolated value.
*/
{

	return (start * (1.0 - weight)) + (end * weight);

};


MVector PositionList::average(const std::vector<PositionListItem>& items)
/**
Returns the weighted average of the supplied position items.

@param items: The position items to average.
@return: Weighted average vector.
*/
{
	
	// Evaluate item count
	//
	unsigned long itemCount = items.size();
	MVector average = MVector(MVector::zero);

	if (itemCount == 0)
	{

		return average;

	}
	
	// Calculate weighted average
	//
	for (PositionListItem item : items)
	{

		// Evaluate which method to use
		//
		if (item.absolute)
		{

			average = lerp(average, item.translate, item.weight);

		}
		else
		{

			average += (item.translate * item.weight);

		}
		
	}
	
	return average;
	
}


void PositionList::normalize(std::vector<PositionListItem>& items)
/**
Normalizes the passed weights so that the total sum equals 1.0.

@param items: The items to normalize.
@return: void
*/
{

	// Get weight sum
	//
	unsigned long itemCount = items.size();
	float sum = 0.0;

	for (unsigned long i = 0; i < itemCount; i++)
	{

		sum += std::fabs(items[i].weight);

	}

	// Check for divide by zero errors!
	//
	if (sum == 0.0 || sum == 1.0)
	{

		return;

	}

	// Multiply weights by scale factor
	//
	float factor = 1.0 / sum;

	for (unsigned long i = 0; i < itemCount; i++)
	{

		items[i].weight *= factor;

	}

};


MMatrix PositionList::createTranslationMatrix(const double x, const double y, const double z)
/**
Returns a position matrix from the supplied XYZ values.

@param x: The X value.
@param x: The Y value.
@param x: The Z value.
@return: The new position matrix.
*/
{

	double matrix[4][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ x, y, z, 1.0 },
	};

	return MMatrix(matrix);

};


MMatrix PositionList::createTranslationMatrix(const MVector& position)
/**
Returns a position matrix from the supplied vector.

@param position: The vector to convert.
@return: The new position matrix.
*/
{

	return PositionList::createTranslationMatrix(position.x, position.y, position.z);

};


void* PositionList::creator() 
/**
This function is called by Maya when a new instance is requested.
See pluginMain.cpp for details.

@return: PositionList
*/
{

	return new PositionList();

};


MStatus PositionList::initialize()
/**
This function is called by Maya after a plugin has been loaded.
Use this function to define any static attributes.

@return: MStatus
*/
{
	
	MStatus status;

	// Initialize function sets
	//
	MFnNumericAttribute fnNumericAttr;
	MFnTypedAttribute fnTypedAttr;
	MFnUnitAttribute fnUnitAttr;
	MFnMatrixAttribute fnMatrixAttr;
	MFnCompoundAttribute fnCompoundAttr;

	// Input attributes:
	// ".active" attribute
	//
	PositionList::active = fnNumericAttr.create("active", "a", MFnNumericData::kInt, 0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// ".normalizeWeights" attribute
	//
	PositionList::normalizeWeights = fnNumericAttr.create("normalizeWeights", "nw", MFnNumericData::kBoolean, false, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// ".name" attribute
	//
	PositionList::name = fnTypedAttr.create("name", "n", MFnData::kString, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnTypedAttr.addToCategory(PositionList::listCategory));

	// ".weight" attribute
	//
	PositionList::weight = fnNumericAttr.create("weight", "w", MFnNumericData::kFloat, 1.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	CHECK_MSTATUS(fnNumericAttr.setMin(-1.0));
	CHECK_MSTATUS(fnNumericAttr.setMax(1.0));
	CHECK_MSTATUS(fnNumericAttr.addToCategory(PositionList::listCategory));

	// ".absolute" attribute
	//
	PositionList::absolute = fnNumericAttr.create("absolute", "abs", MFnNumericData::kBoolean, false, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnNumericAttr.addToCategory(PositionList::listCategory));

	// ".translateX" attribute
	//
	PositionList::translateX = fnUnitAttr.create("translateX", "tx", MFnUnitAttribute::kDistance, 0.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::translateCategory));
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::listCategory));

	// ".translateY" attribute
	//
	PositionList::translateY = fnUnitAttr.create("translateY", "ty",  MFnUnitAttribute::kDistance, 0.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::translateCategory));
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::listCategory));

	// ".translateZ" attribute
	//
	PositionList::translateZ = fnUnitAttr.create("translateZ", "tz",  MFnUnitAttribute::kDistance, 0.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::translateCategory));
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::listCategory));

	// ".translate" attribute
	//
	PositionList::translate = fnNumericAttr.create("translate", "t", PositionList::translateX, PositionList::translateY, PositionList::translateZ, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::translateCategory));
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::listCategory));

	// ".list" attribute
	//
	PositionList::list = fnCompoundAttr.create("list", "l", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	
	CHECK_MSTATUS(fnCompoundAttr.addChild(PositionList::name));
	CHECK_MSTATUS(fnCompoundAttr.addChild(PositionList::weight));
	CHECK_MSTATUS(fnCompoundAttr.addChild(PositionList::absolute));
	CHECK_MSTATUS(fnCompoundAttr.addChild(PositionList::translate));
	CHECK_MSTATUS(fnCompoundAttr.setArray(true));
	CHECK_MSTATUS(fnCompoundAttr.addToCategory(PositionList::listCategory));

	// Output attributes:
	// ".outputX" attribute
	//
	PositionList::outputX = fnUnitAttr.create("outputX", "ox", MFnUnitAttribute::kDistance, 0.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnUnitAttr.setWritable(false));
	CHECK_MSTATUS(fnUnitAttr.setStorable(false));
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::outputCategory));

	// ".outputY" attribute
	//
	PositionList::outputY = fnUnitAttr.create("outputY", "oy", MFnUnitAttribute::kDistance, 0.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnUnitAttr.setWritable(false));
	CHECK_MSTATUS(fnUnitAttr.setStorable(false));
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::outputCategory));

	// ".outputZ" attribute
	//
	PositionList::outputZ = fnUnitAttr.create("outputZ", "oz", MFnUnitAttribute::kDistance, 0.0, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnUnitAttr.setWritable(false));
	CHECK_MSTATUS(fnUnitAttr.setStorable(false));
	CHECK_MSTATUS(fnUnitAttr.addToCategory(PositionList::outputCategory));

	// ".output" attribute
	//
	PositionList::output = fnNumericAttr.create("output", "o", PositionList::outputX, PositionList::outputY, PositionList::outputZ, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnNumericAttr.setWritable(false));
	CHECK_MSTATUS(fnNumericAttr.setStorable(false));
	CHECK_MSTATUS(fnNumericAttr.addToCategory(PositionList::outputCategory));

	// ".matrix" attribute
	//
	PositionList::matrix = fnMatrixAttr.create("matrix", "m", MFnMatrixAttribute::kDouble, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnMatrixAttr.setWritable(false));
	CHECK_MSTATUS(fnMatrixAttr.setStorable(false));
	CHECK_MSTATUS(fnMatrixAttr.addToCategory(PositionList::outputCategory));

	// ".inverseMatrix" attribute
	//
	PositionList::inverseMatrix = fnMatrixAttr.create("inverseMatrix", "im", MFnMatrixAttribute::kDouble, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	CHECK_MSTATUS(fnMatrixAttr.setWritable(false));
	CHECK_MSTATUS(fnMatrixAttr.setStorable(false));
	CHECK_MSTATUS(fnMatrixAttr.addToCategory(PositionList::outputCategory));

	// Add attributes to node
	//
	CHECK_MSTATUS(PositionList::addAttribute(PositionList::active));
	CHECK_MSTATUS(PositionList::addAttribute(PositionList::normalizeWeights));
	CHECK_MSTATUS(PositionList::addAttribute(PositionList::list));

	CHECK_MSTATUS(PositionList::addAttribute(PositionList::output));
	CHECK_MSTATUS(PositionList::addAttribute(PositionList::matrix));
	CHECK_MSTATUS(PositionList::addAttribute(PositionList::inverseMatrix));

	// Define attribute relationships
	//
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::active, PositionList::outputX));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::normalizeWeights, PositionList::outputX));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::weight, PositionList::outputX));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::absolute, PositionList::outputX));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateX, PositionList::outputX));

	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::active, PositionList::outputY));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::normalizeWeights, PositionList::outputY));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::weight, PositionList::outputY));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::absolute, PositionList::outputY));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateY, PositionList::outputY));

	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::active, PositionList::outputZ));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::normalizeWeights, PositionList::outputZ));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::weight, PositionList::outputZ));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::absolute, PositionList::outputZ));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateZ, PositionList::outputZ));

	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::active, PositionList::matrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::normalizeWeights, PositionList::matrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::weight, PositionList::matrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::absolute, PositionList::matrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateX, PositionList::matrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateY, PositionList::matrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateZ, PositionList::matrix));

	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::active, PositionList::inverseMatrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::normalizeWeights, PositionList::inverseMatrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::weight, PositionList::inverseMatrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::absolute, PositionList::inverseMatrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateX, PositionList::inverseMatrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateY, PositionList::inverseMatrix));
	CHECK_MSTATUS(PositionList::attributeAffects(PositionList::translateZ, PositionList::inverseMatrix));

	return status;

};