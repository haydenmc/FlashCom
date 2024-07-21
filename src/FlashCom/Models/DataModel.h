#pragma once
#include "TreeNode.h"

namespace FlashCom::Models
{
    struct DataModel
    {
        std::string LoadErrorMessage;
        std::shared_ptr<TreeNode> RootNode;
        TreeNode* CurrentNode{ nullptr };
    };
}
