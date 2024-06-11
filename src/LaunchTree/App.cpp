#include "App.h"
#include <vector>

namespace
{
    std::unique_ptr<LaunchTree::Models::DataModel> CreateDataModel()
    {
        std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>> rootChildren;
        rootChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
            'C',
            L"Comms",
            std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
        ));
        rootChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
            'D',
            L"Dev",
            std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
        ));
        rootChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
            'G',
            L"Gaming",
            std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
        ));

        auto rootNode{
            std::make_unique<LaunchTree::Models::TreeNode>(
                '\0',
                L"",
                std::move(rootChildren)
            )
        };

        return std::make_unique<LaunchTree::Models::DataModel>(
            std::move(rootNode));
    }
}

namespace LaunchTree
{
#pragma region Public
    std::unique_ptr<App> App::CreateApp()
    {
        return std::unique_ptr<App>(new App(std::move(CreateDataModel())));
    }
#pragma endregion Public
#pragma region Private
    App::App(std::unique_ptr<Models::DataModel>&& dataModel) : 
        m_dataModel{ std::move(dataModel) }
    { }
#pragma endregion Private
}