#ifndef CNOID_MANIPULATOR_PLUGIN_MANIPULATOR_VARIABLE_LIST_ITEM_H
#define CNOID_MANIPULATOR_PLUGIN_MANIPULATOR_VARIABLE_LIST_ITEM_H

#include <cnoid/Item>
#include "exportdecl.h"

namespace cnoid {

class ManipulatorVariableList;

class CNOID_EXPORT ManipulatorVariableListItem : public Item
{
public:
    static void initializeClass(ExtensionManager* ext);

    ManipulatorVariableListItem();
    ManipulatorVariableListItem(const ManipulatorVariableListItem& org);
    virtual ~ManipulatorVariableListItem();

    ManipulatorVariableList* variableList();
    const ManipulatorVariableList* variableList() const;

    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<ManipulatorVariableListItem> ManipulatorVariableListItemPtr;

}

#endif
