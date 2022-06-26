//
// File: pluginMain.cpp
//
// Author: Benjamin H. Singleton
//

#include "PositionListNode.h"
#include <maya/MFnPlugin.h>


MStatus initializePlugin(MObject obj) 
{

	MStatus status;

	MFnPlugin plugin(obj, "Ben Singleton", "2020", "Any");
	status = plugin.registerNode("positionList", PositionList::id, PositionList::creator, PositionList::initialize);
	
	if (!status) 
	{

		status.perror("registerNode");
		return status;

	}

	return status;

}

MStatus uninitializePlugin(MObject obj) 
{

	MStatus status;

	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(PositionList::id);

	if (!status) 
	{

		status.perror("deregisterNode");
		return status;

	}

	return status;

}
