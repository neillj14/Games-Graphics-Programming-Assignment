/*
Filename: RubixCubeAppInfo.cpp
Description: Implementation file for RubixCubeAppInfo.h
Author: Jamie Neill
Copyright: Ulster University
Date: 15/04/2019
*/

#include "RubixCubeAppInfo.h"



RubixCubeAppInfo::RubixCubeAppInfo()
{
	//Initialise the info class with blank values so as not to trip any switch case statements that aren't intended
    needsReset_ = false;
    rotationAxis_ = ' ';
    renderingMode_ = ' ';
    cullMode_ = ' ';
    cameraPosition_ = 0;
    selectedThird_ = ' ';
    rotatable_ = false;
    rotationAxis_ = ' ';
    topIndicies_ = { 6,7,8,15,16,17,24,25,26 };
    bottomIndicies_ = { 0,1,2,9,10,11,18,19,20 };
    leftIndicies_ = { 0,1,2,3,4,5,6,7,8 };
    rightIndicies_ = { 18,19,20,21,22,23,24,25,26 };
    backIndicies_ = { 2,5,8,11,14,17,20,23,26 };
    frontIndicies_ = { 0,3,6,9,12,15,18,21,24 };

}


RubixCubeAppInfo::~RubixCubeAppInfo()
{
}

//Getters and setters
bool RubixCubeAppInfo::needsReset() const
{
    return needsReset_;
}

char RubixCubeAppInfo::rotationAxis() const
{
    return rotationAxis_;
}

char RubixCubeAppInfo::getFill() const
{
    return renderingMode_;
}

char RubixCubeAppInfo::getCull() const
{
    return cullMode_;
}

int RubixCubeAppInfo::getCameraPosition() const
{
    return cameraPosition_;
}

char RubixCubeAppInfo::getSelectedThird() const
{
    return selectedThird_;
}

bool RubixCubeAppInfo::getRotatable() const
{
    return rotatable_;
}

void RubixCubeAppInfo::needsReset(bool needsReset)
{
    needsReset_ = needsReset;
}

void RubixCubeAppInfo::setRotationAxis(char rotationAxis)
{
    rotationAxis_ = rotationAxis;
}

void RubixCubeAppInfo::setRenderMode(char renderingMode)
{
    renderingMode_ = renderingMode;
}

void RubixCubeAppInfo::setCullMode(char cullMode)
{
    cullMode_ = cullMode;
}

void RubixCubeAppInfo::setCameraPosition(int cameraPosition)
{
    cameraPosition_ = cameraPosition;
}

void RubixCubeAppInfo::setSelectedThird(char selectedThird)
{
    selectedThird_ = selectedThird;
}

void RubixCubeAppInfo::setRotatable(bool rotatable)
{
    rotatable_ = rotatable;
}
