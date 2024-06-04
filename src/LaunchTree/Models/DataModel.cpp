#include <pch.h>
#include "DataModel.h"

namespace LaunchTree::Models
{
    DataModel::DataModel(std::unique_ptr<TreeNode>&& rootNode) :
        RootNode{ std::move(rootNode) }
    { }
}