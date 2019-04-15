/*Filename: RubixCubeAppInfo.h
 Description: Definition file for RubixCubeAppInfo.cpp intended to
 store all the relevant user inputs to theCOM428 Assignment
 Author: Jamie Neill
 Date: 15/04/2019
 Copyright: Ulster University*/

#pragma once
#include <vector>
#include"Common/d3dUtil.h"
class RubixCubeAppInfo
{
public:
    RubixCubeAppInfo();
    ~RubixCubeAppInfo();
	//Getters
    bool needsReset()const;
    char rotationAxis()const;
    char getFill()const;
    char getCull()const;
    int getCameraPosition()const;
    char getSelectedThird()const;
    bool getRotatable() const;

	//Setters
    void needsReset(bool needsReset);
    void setRotationAxis(char rotationAxis);
    void setRenderMode(char renderingMode);
    void setCullMode(char cullMode);
    void setCameraPosition(int cameraPosition);
    void setSelectedThird(char selectedThird);
    void setRotatable(bool rotatable);

	//Vectrors storing the constant buffers of the cubes making up each face
    std::vector<UINT> topIndicies_;
    std::vector<UINT> bottomIndicies_;
    std::vector<UINT> leftIndicies_;
    std::vector<UINT> rightIndicies_;
    std::vector<UINT> backIndicies_;
    std::vector<UINT> frontIndicies_;

private:
    bool needsReset_;
    char rotationAxis_;
    char renderingMode_;
    char cullMode_;
    int cameraPosition_;
    char selectedThird_;
    bool rotatable_;
};