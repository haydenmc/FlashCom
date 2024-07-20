#pragma once
#include "TreeNode.h"

namespace FlashCom::Models
{
    struct DataModel
    {
        DataModel(std::unique_ptr<TreeNode>&& rootNode);
        DataModel(DataModel&& other) = default;

        const std::unique_ptr<TreeNode> RootNode;
        TreeNode* CurrentNode{ nullptr };
    };
}
