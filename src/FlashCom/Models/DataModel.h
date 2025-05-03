#pragma once
#include "TreeNode.h"
#include <stack>

namespace FlashCom::Models
{
    struct DataModel
    {
        std::string LoadErrorMessage;
        bool ShowStartupNotification;
        bool UseTwentyFourHourClock;
        std::shared_ptr<TreeNode> RootNode;
        std::stack<TreeNode*> CurrentNodeStack;

        void ResetCurrentNode()
        {
            std::stack<TreeNode*> newStack;
            newStack.push(RootNode.get());
            CurrentNodeStack.swap(newStack);
        }

        const std::vector<TreeNode*> CurrentNodeChildren() const
        {
            if (CurrentNodeStack.empty())
            {
                return {};
            }
            return CurrentNodeStack.top()->GetChildren();
        }
    };
}
