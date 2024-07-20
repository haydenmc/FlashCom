#include <pch.h>
#include "DataModel.h"

namespace FlashCom::Models
{
    DataModel::DataModel(std::unique_ptr<TreeNode>&& rootNode) :
        RootNode{ std::move(rootNode) },
        CurrentNode{ RootNode.get() }
    { }
}