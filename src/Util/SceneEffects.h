/*!
  @file
  @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_UTIL_SCENE_EFFECTS_H
#define CNOID_UTIL_SCENE_EFFECTS_H

#include "SceneGraph.h"
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT SgFog : public SgPreprocessed
{
protected:
    SgFog(int polymorhicId);
    
public:
    SgFog();
    SgFog(const SgFog& org);

    const Vector3f& color() const { return color_; }
    template<typename Derived> void setColor(const Eigen::MatrixBase<Derived>& c) {
        color_ = c.template cast<Vector3f::Scalar>(); }
    void setVisibilityRange(float r) { visibilityRange_ = r; }
    float visibilityRange() const { return visibilityRange_; }

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;
    
private:
    Vector3f color_;
    float  visibilityRange_;
    //int fogType;
};

typedef ref_ptr<SgFog> SgFogPtr;


class CNOID_EXPORT SgOutline : public SgGroup
{
protected:
    SgOutline(int polymorhicId);
    
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    SgOutline();

    const Vector3f& color() const { return color_; }
    void setColor(const Vector3f& color) { color_ = color; }
    void setLineWidth(float width) { lineWidth_ = width; }
    float lineWidth() const { return lineWidth_; }

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;

private:
    Vector3f color_;
    float lineWidth_;
};

typedef ref_ptr<SgOutline> SgOutlinePtr;


class CNOID_EXPORT SgLightweightRenderingGroup : public SgGroup
{
public:
    SgLightweightRenderingGroup();

protected:
    virtual Referenced* doClone(CloneMap* cloneMap) const override;
};

typedef ref_ptr<SgLightweightRenderingGroup> SgLightweightRenderingGroupPtr;

}

#endif
