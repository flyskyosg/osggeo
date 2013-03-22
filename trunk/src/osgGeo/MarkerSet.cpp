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

$Id: MarkerSet.cpp 108 2012-12-08 08:32:40Z kristofer.tingdahl@dgbes.com $

*/

#include "osgGeo/MarkerSet"
#include <osgGeo/AutoTransform>

namespace osgGeo
{
    MarkerSet::MarkerSet()
	:_rotateMode(osgGeo::AutoTransform::ROTATE_TO_SCREEN)
	,_hints(new osg::TessellationHints)
	,_shapeType(osgGeo::MarkerSet::Box)
	,_colorArr(new osg::Vec4Array)
	,_vertexArr(new osg::Vec3Array)
	,_nonShadingGroup(new osg::Group)
	,_needsUpdate(true)
	,_arrayModCount(-1)
	,_radius(.0)
	,_height(.0)
	,_minScale(.0)
	,_maxScale(.0)
	,_normalArr(new osg::Vec3Array)
    {
	setNumChildrenRequiringUpdateTraversal(1);
	_hints->setDetailRatio(0.5f);
    }


     MarkerSet::~MarkerSet()
    {
    }


    void MarkerSet::traverse(osg::NodeVisitor& nv)
    {
	if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
	{
	    if (needsUpdate())
		updateShapes();
	}
	if (_nonShadingGroup) 
	    _nonShadingGroup->accept(nv);
    }


    bool MarkerSet::updateShapes()
    {
	if (!_vertexArr || !_needsUpdate)
	    return false;

	_nonShadingGroup->removeChildren(0, _nonShadingGroup->getNumChildren());

	for (unsigned int idx=0;idx<_vertexArr->size();idx++)
	{
	    osg::ref_ptr<osg::ShapeDrawable> shapeDrwB;
	    osg::ref_ptr<osgGeo::AutoTransform> autotrans = 
		new osgGeo::AutoTransform;
	    switch (_shapeType)
	    {
	    case osgGeo::MarkerSet::Box:
		shapeDrwB = new osg::ShapeDrawable(new osg::Box(
				osg::Vec3f(0,0,0),_radius),_hints);
		break;
	    case osgGeo::MarkerSet::Cone:
		shapeDrwB = new osg::ShapeDrawable(new osg::Cone(
				osg::Vec3f(0,0,0),_radius,_height),_hints);
		break;
	    case osgGeo::MarkerSet::Sphere:
		shapeDrwB = new osg::ShapeDrawable(new osg::Sphere(
				osg::Vec3f(0,0,0),_radius),_hints);
		break;
	    case osgGeo::MarkerSet::Cylinder:
		shapeDrwB = new osg::ShapeDrawable(new osg::Cylinder(
				osg::Vec3f(0,0,0),_radius,_height),_hints);
		break;
	    default:
		return false;
	    }

	    if (idx<_colorArr->size())
		shapeDrwB->setColor(_colorArr->at( idx ));
	    else
		shapeDrwB->setColor(*(_colorArr->end()-1));

	    autotrans->setPosition(_vertexArr->at(idx));
	    autotrans->setAutoRotateMode(_rotateMode);
	    autotrans->setMinimumScale(_minScale);
	    autotrans->setMaximumScale(_maxScale);
	    autotrans->setAutoScaleToScreen(true);
	    autotrans->setRestoreProportions(true);

	    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	    geode->addDrawable(shapeDrwB);
	    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,
						  osg::StateAttribute::ON);
	    autotrans->addChild(geode);
	    _nonShadingGroup->addChild(autotrans);
	}
	_needsUpdate = false;
	_arrayModCount = _vertexArr->getModifiedCount();
	return true;
    }


    osg::BoundingSphere MarkerSet::computeBound() const
    {
	osg::BoundingSphere sphere;
	for (std::vector<osg::Vec3>::const_iterator iter = _vertexArr->begin();
	    iter != _vertexArr->end();
	    iter++)
	{
	    sphere.expandBy(*iter);
	}

	return sphere;
    }


    bool MarkerSet::needsUpdate()
    {
	if ((int)_vertexArr->getModifiedCount()>_arrayModCount)
	    _needsUpdate = true;
	return _needsUpdate;
    }


    void MarkerSet::setVertexArray(osg::Vec3Array* arr)
    {
	if (arr==_vertexArr)
	    return;
	_vertexArr = arr;
	_needsUpdate = true;
    }


    void MarkerSet::setNormalArray(osg::Vec3Array* arr)
    {
	if (arr==_normalArr)
	    return;
	_normalArr = arr;
	_needsUpdate = true;
    }


    void MarkerSet::setShape(osgGeo::MarkerSet::MarkerType shape)
    {
	_shapeType = shape;
	_needsUpdate = true;
    }


    void MarkerSet::setRadius(float radius)
    {
	_radius = radius;
	_needsUpdate = true;

    }


    void MarkerSet::setHeight(float height)
    {
	_height = height;
	_needsUpdate = true;
    }


    void MarkerSet::setDetail(float ratio)
    {
	_hints->setDetailRatio(ratio);
	_needsUpdate = true;
    }


    void MarkerSet::setMinScale(double minscale)
    {
	_minScale = minscale;
	_needsUpdate = true;
    }


    void MarkerSet::setMaxScale(double maxscale)
    {
	_maxScale = maxscale;
	_needsUpdate = true;
    }


    void MarkerSet::setRotateMode(osg::AutoTransform::AutoRotateMode rtmode)
    {
	_rotateMode = rtmode;
	_needsUpdate = true;
    }


    void MarkerSet::setColorArray(osg::Vec4Array* colorArr)
    {
	_colorArr = colorArr;
	_needsUpdate = true;
    }

}