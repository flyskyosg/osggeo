/* osgGeo - A collection of geoscientific extensions to OpenSceneGraph.
Copyright 2011 dGB Beheer B.V.

osgGeo is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


#include <osg/Geometry>

#include <osgGeo/Horizon3D>

#include <iostream>

namespace osgGeo
{

Horizon3DNode::Horizon3DNode() :
    Geode()
{

}

Horizon3DNode::Horizon3DNode(const Horizon3DNode& other,
              const osg::CopyOp& op) :
    Geode(other, op)
{

}

void Horizon3DNode::setSize(const Vec2i &size)
{
    _size = size;
}

Vec2i Horizon3DNode::getSize() const
{
    return _size;
}

void Horizon3DNode::setDepthArray(osg::Array *arr)
{
    _array = arr;
    updateDrawables();
}

const osg::Array *Horizon3DNode::getDepthArray() const
{
    return _array;
}

osg::Array *Horizon3DNode::getDepthArray()
{
    return _array;
}

void Horizon3DNode::updateDrawables()
{
    if(getDepthArray()->getType() != osg::Array::DoubleArrayType)
        return;

    osg::DoubleArray &depthVals =
            *(dynamic_cast<osg::DoubleArray*>(getDepthArray()));

    const int allHSize = getSize().x();
    const int allVSize = getSize().y();

    const int hSize = allHSize;
    const int vSize = allVSize;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(hSize * vSize);

    // first we construct an array of vertices which is just a grid
    // of depth values.
    for(int i = 0; i < hSize; ++i)
        for(int j = 0; j < vSize; ++j)
        {
            float val = depthVals[i*allVSize+j];
            if(isUndef(val))
                val = 0.0;

            (*vertices)[i*vSize+j] = osg::Vec3(
                        float(i) / hSize,
                        float(j) / vSize,
                        val
                        );
        }

    // the following loop populates array of indices that make up
    // triangles out of vertices data, each grid cell has 2 triangles.
    // If a vertex is undefined then triangle that contains it is
    // discarded. Also normals are calculated for each triangle to be
    // later used for calculating normals per vertex
    osg::ref_ptr<osg::DrawElementsUInt> indices =
            new osg::DrawElementsUInt(GL_TRIANGLES);

    osg::ref_ptr<osg::Vec3Array> triangleNormals =
            new osg::Vec3Array((hSize - 1) * (vSize - 1) * 2);

    for(int i = 0; i < hSize - 1; ++i)
        for(int j = 0; j < vSize - 1; ++j)
        {
            float val00 = depthVals[i*allVSize+j];
            float val10 = depthVals[(i+1)*allVSize+j];
            float val01 = depthVals[i*allVSize+(j+1)];
            float val11 = depthVals[(i+1)*allVSize+(j+1)];

            if(isUndef(val10) || isUndef(val01))
                continue;

            int i00 = i*vSize+j;
            int i10 = (i+1)*vSize+j;
            int i01 = i*vSize+(j+1);
            int i11 = (i+1)*vSize+(j+1);

            // first triangle
            if(!isUndef(val00))
            {
                indices->push_back(i00);
                indices->push_back(i10);
                indices->push_back(i01);
            }

            // second triangle
            if(!isUndef(val11))
            {
                indices->push_back(i10);
                indices->push_back(i01);
                indices->push_back(i11);
            }

            // calculate triangle normals
            osg::Vec3 v00 = (*vertices)[i00];
            osg::Vec3 v10 = (*vertices)[i10];
            osg::Vec3 v01 = (*vertices)[i01];
            osg::Vec3 v11 = (*vertices)[i11];

            osg::Vec3 norm1 = (v01 - v00) ^ (v10 - v00);
            norm1.normalize();
            (*triangleNormals)[(i*(vSize-1)+j)*2] = norm1;

            osg::Vec3 norm2 = (v10 - v11) ^ (v01 - v11);
            norm2.normalize();
            (*triangleNormals)[(i*(vSize-1)+j)*2+1] = norm2;
        }

    // The following loop calculates normals per vertex. Because
    // each vertex might be shared between many triangles(up to 6)
    // we find out which triangles this particular vertex is shared
    // and then compute the average of normals per triangle.
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(hSize * vSize);

    osg::Vec3 triNormCache[6];

    for(int i = 0; i < hSize; ++i)
        for(int j = 0; j < vSize; ++j)
        {
            int k = 0;
            for(int l = 0; l < k; ++l)
                triNormCache[l] = osg::Vec3();

            if((i < hSize - 1) && (j < vSize - 1))
            {
                triNormCache[k++] = (*triangleNormals)[(i*(vSize-1)+j)*2];
            }

            if(i > 0 && j < vSize - 1)
            {
                triNormCache[k++] = (*triangleNormals)[((i-1)*(vSize-1)+j)*2];
                triNormCache[k++] = (*triangleNormals)[((i-1)*(vSize-1)+j)*2+1];
            }

            if(j > 0 && i < hSize - 1)
            {
                triNormCache[k++] = (*triangleNormals)[(i*(vSize-1)+j-1)*2];
                triNormCache[k++] = (*triangleNormals)[(i*(vSize-1)+j-1)*2+1];
            }

            if(i > 0 && j > 0)
            {
                triNormCache[k++] = (*triangleNormals)[((i-1)*(vSize-1)+j-1)*2+1];
            }

            if(k > 0)
            {
                osg::Vec3 norm;
                for(int l = 0; l < k; ++l)
                    norm += triNormCache[l];

                norm /= k;
                norm.normalize();

                (*normals)[i*vSize+j] = -norm;
            }
        }

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f, 0.0f, 1.0f, 1.0f));

    osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
    quad->setVertexArray(vertices.get());

    quad->setNormalArray(normals.get());
    quad->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    quad->setColorArray(colors.get());
    quad->setColorBinding(osg::Geometry::BIND_OVERALL);

    quad->addPrimitiveSet(indices.get());

    addDrawable(quad.get());
}

bool Horizon3DNode::isUndef(double val)
{
    return val >= _maxdepth;
}

void Horizon3DNode::setMaxDepth(float val)
{
    _maxdepth = val;
}

float Horizon3DNode::getMaxDepth() const
{
    return _maxdepth;
}

}