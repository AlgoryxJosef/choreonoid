/**
   @author Shin'ichiro Nakaoka
*/

#include "BodyBar.h"
#include "BodyItem.h"
#include <cnoid/RootItem>
#include <cnoid/Archive>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BodyBar::Impl
{
public:
    BodyItemPtr currentBodyItem;
    ItemList<BodyItem> selectedBodyItems;
    ItemList<BodyItem> targetBodyItems;
    Connection connectionOfItemSelectionChanged;
    Connection connectionOfCurrentBodyItemDetachedFromRoot;
    Signal<void(const ItemList<BodyItem>& selectedBodyItems)> sigBodyItemSelectionChanged;
    Signal<void(BodyItem* currentBodyItem)> sigCurrentBodyItemChanged;

    Impl(BodyBar* self);
    ~Impl();
    bool makeSingleSelection(BodyItem* bodyItem);
    void onSelectedItemsChanged(ItemList<BodyItem> bodyItems);
    void onBodyItemDetachedFromRoot();
    void onCopyButtonClicked();
    void onPasteButtonClicked();
    void onOriginButtonClicked();
    void onPoseButtonClicked(BodyItem::PresetPoseID id);
    void onSymmetricCopyButtonClicked(int direction, bool doMirrorCopy);
    bool restoreState(const Archive& archive);
};

}


BodyBar* BodyBar::instance()
{
    static BodyBar* instance = new BodyBar();
    return instance;
}


BodyBar::BodyBar()
    : ToolBar(N_("BodyBar"))
{
    impl = new Impl(this);
}


BodyBar::Impl::Impl(BodyBar* self)
{
    self->setVisibleByDefault(true);

    self->addButton(QIcon(":/Body/icons/storepose.png"), _("Memory the current pose"))
        ->sigClicked().connect([&](){ onCopyButtonClicked(); });

    self->addButton(QIcon(":/Body/icons/restorepose.png"), _("Recall the memorized pose"))
        ->sigClicked().connect([&](){ onPasteButtonClicked(); });
    
    self->addButton(QIcon(":/Body/icons/origin.png"), _("Move the selected bodies to the origin"))
        ->sigClicked().connect([&](){ onOriginButtonClicked(); });

    self->addButton(QIcon(":/Body/icons/initialpose.png"), _("Set the preset initial pose to the selected bodies"))
        ->sigClicked().connect([&](){ onPoseButtonClicked(BodyItem::INITIAL_POSE); });

    self->addButton(QIcon(":/Body/icons/stdpose.png"), _("Set the preset standard pose to the selected bodies"))
        ->sigClicked().connect([&](){ onPoseButtonClicked(BodyItem::STANDARD_POSE); });

    self->addSeparator();

    self->addButton(QIcon(":/Body/icons/right-to-left.png"), _("Copy the right side pose to the left side"))
        ->sigClicked().connect([&](){ onSymmetricCopyButtonClicked(1, false); });

    self->addButton(QIcon(":/Body/icons/flip.png"), _("Mirror copy"))
        ->sigClicked().connect([&](){ onSymmetricCopyButtonClicked(0, true); });

    self->addButton(QIcon(":/Body/icons/left-to-right.png"), _("Copy the left side pose to the right side"))
        ->sigClicked().connect([&](){ onSymmetricCopyButtonClicked(0, false); });

    connectionOfItemSelectionChanged = 
        RootItem::instance()->sigSelectedItemsChanged().connect(
            [&](const ItemList<>& items){ onSelectedItemsChanged(items); });
}


BodyBar::~BodyBar()
{
    delete impl;
}


BodyBar::Impl::~Impl()
{
    connectionOfItemSelectionChanged.disconnect();
    connectionOfCurrentBodyItemDetachedFromRoot.disconnect();
}


SignalProxy<void(const ItemList<BodyItem>& selectedBodyItems)> BodyBar::sigBodyItemSelectionChanged()
{
    return impl->sigBodyItemSelectionChanged;
}


SignalProxy<void(BodyItem* currentBodyItem)> BodyBar::sigCurrentBodyItemChanged()
{
    return impl->sigCurrentBodyItemChanged;
}


const ItemList<BodyItem>& BodyBar::selectedBodyItems()
{
    return impl->selectedBodyItems;
}


const ItemList<BodyItem>& BodyBar::targetBodyItems()
{
    return impl->targetBodyItems;
}


BodyItem* BodyBar::currentBodyItem()
{
    return impl->currentBodyItem;
}


bool BodyBar::makeSingleSelection(BodyItem* bodyItem)
{
    return impl->makeSingleSelection(bodyItem);
}


bool BodyBar::Impl::makeSingleSelection(BodyItem* bodyItem)
{
    ItemList<BodyItem> prevSelected = selectedBodyItems;
    for(size_t i=0; i < prevSelected.size(); ++i){
        BodyItem* item = prevSelected[i];
        if(item != bodyItem){
            item->setSelected(false);
        }
    }
    bodyItem->setSelected(true);
    return true;
}


void BodyBar::Impl::onSelectedItemsChanged(ItemList<BodyItem> bodyItems)
{
    bool selectedBodyItemsChanged = false;
    
    if(selectedBodyItems != bodyItems){
        selectedBodyItems = bodyItems;
        selectedBodyItemsChanged = true;
    }

    BodyItem* firstItem = bodyItems.toSingle();

    if(firstItem && firstItem != currentBodyItem){
        currentBodyItem = firstItem;
        connectionOfCurrentBodyItemDetachedFromRoot.disconnect();
        connectionOfCurrentBodyItemDetachedFromRoot =
            currentBodyItem->sigDetachedFromRoot().connect(
                [&](){ onBodyItemDetachedFromRoot(); });
        sigCurrentBodyItemChanged(currentBodyItem);
    }

    if(selectedBodyItemsChanged){
        sigBodyItemSelectionChanged(selectedBodyItems);
    }

    targetBodyItems.clear();
    if(selectedBodyItems.empty()){
        if(currentBodyItem){
            targetBodyItems.push_back(currentBodyItem);
        }
    } else {
        targetBodyItems = selectedBodyItems;
    }
}


void BodyBar::Impl::onCopyButtonClicked()
{
    if(currentBodyItem){
        currentBodyItem->copyKinematicState();
    }
}


void BodyBar::Impl::onPasteButtonClicked()
{
    for(size_t i=0; i < targetBodyItems.size(); ++i){
        targetBodyItems[i]->pasteKinematicState();
    }
}


void BodyBar::Impl::onBodyItemDetachedFromRoot()
{
    currentBodyItem = 0;
    connectionOfCurrentBodyItemDetachedFromRoot.disconnect();
    sigCurrentBodyItemChanged(nullptr);
}


void BodyBar::Impl::onOriginButtonClicked()
{
    for(size_t i=0; i < targetBodyItems.size(); ++i){
        targetBodyItems[i]->moveToOrigin();
    }
}


void BodyBar::Impl::onPoseButtonClicked(BodyItem::PresetPoseID id)
{
    for(size_t i=0; i < targetBodyItems.size(); ++i){
        targetBodyItems[i]->setPresetPose(id);
    }
}


void BodyBar::Impl::onSymmetricCopyButtonClicked(int direction, bool doMirrorCopy)
{
    for(size_t i=0; i < targetBodyItems.size(); ++i){
        const Listing& slinks = *targetBodyItems[i]->body()->info()->findListing("symmetricJoints");
        if(slinks.isValid() && !slinks.empty()){
            targetBodyItems[i]->beginKinematicStateEdit();
            int from = direction;
            int to = 1 - direction;
            BodyPtr body = targetBodyItems[i]->body();
            for(int j=0; j < slinks.size(); ++j){
                const Listing& linkPair = *slinks[j].toListing();
                if(linkPair.size() == 1 && doMirrorCopy){
                    Link* link = body->link(linkPair[0].toString());
                    if(link){
                        link->q() = -link->q();
                    }
                } else if(linkPair.size() >= 2){
                    Link* link1 = body->link(linkPair[from].toString());
                    Link* link2 = body->link(linkPair[to].toString());
                    if(link1 && link2){
                        double sign = 1.0;
                        
                        if(linkPair.size() >= 3){
                            sign = linkPair[2].toDouble();
                        }
                        if(doMirrorCopy){
                            double q1 = link1->q();
                            link1->q() = sign * link2->q();
                            link2->q() = sign * q1;
                        } else {
                            link2->q() = sign * link1->q();
                        }
                    }
                }
            }
            targetBodyItems[i]->notifyKinematicStateChange(true);
            targetBodyItems[i]->acceptKinematicStateEdit();
        }
    }
}


bool BodyBar::storeState(Archive& archive)
{
    if(impl->currentBodyItem){
        archive.writeItemId("current", impl->currentBodyItem);
    }
    return true;
}


bool BodyBar::restoreState(const Archive& archive)
{
    archive.addPostProcess([&](){ impl->restoreState(archive); });
    return true;
}


bool BodyBar::Impl::restoreState(const Archive& archive)
{
    if(!currentBodyItem){
        currentBodyItem = archive.findItem<BodyItem>("current");
        if(currentBodyItem){
            if(targetBodyItems.empty()){
                targetBodyItems.push_back(currentBodyItem);
            }
            sigCurrentBodyItemChanged(currentBodyItem);
        }
    }
    return true;
}
