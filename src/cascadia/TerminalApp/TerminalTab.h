// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once
#include "Pane.h"
#include "ColorPickupFlyout.h"
#include "TabBase.h"
#include "TerminalTab.g.h"

static constexpr double HeaderRenameBoxWidthDefault{ 165 };
static constexpr double HeaderRenameBoxWidthTitleLength{ std::numeric_limits<double>::infinity() };

// fwdecl unittest classes
namespace TerminalAppLocalTests
{
    class TabTests;
};

namespace winrt::TerminalApp::implementation
{
    struct TerminalTab : TerminalTabT<TerminalTab, TabBase>
    {
    public:
        TerminalTab(const winrt::Microsoft::Terminal::Settings::Model::Profile& profile, const winrt::Microsoft::Terminal::Control::TermControl& control);
        TerminalTab(std::shared_ptr<Pane> rootPane);

        // Called after construction to perform the necessary setup, which relies on weak_ptr
        void Initialize();

        winrt::Microsoft::Terminal::Control::TermControl GetActiveTerminalControl() const;
        winrt::Microsoft::Terminal::Settings::Model::Profile GetFocusedProfile() const noexcept;

        void Focus(winrt::Windows::UI::Xaml::FocusState focusState) override;

        winrt::fire_and_forget Scroll(const int delta);

        std::shared_ptr<Pane> DetachRoot();
        std::shared_ptr<Pane> DetachPane();
        void AttachPane(std::shared_ptr<Pane> pane);

        void SplitPane(winrt::Microsoft::Terminal::Settings::Model::SplitState splitType,
                       const float splitSize,
                       const winrt::Microsoft::Terminal::Settings::Model::Profile& profile,
                       winrt::Microsoft::Terminal::Control::TermControl& control);

        void ToggleSplitOrientation();
        winrt::fire_and_forget UpdateIcon(const winrt::hstring iconPath);
        winrt::fire_and_forget HideIcon(const bool hide);

        winrt::fire_and_forget ShowBellIndicator(const bool show);
        winrt::fire_and_forget ActivateBellIndicatorTimer();

        float CalcSnappedDimension(const bool widthOrHeight, const float dimension) const;
        winrt::Microsoft::Terminal::Settings::Model::SplitState PreCalculateAutoSplit(winrt::Windows::Foundation::Size rootSize) const;
        bool PreCalculateCanSplit(winrt::Microsoft::Terminal::Settings::Model::SplitState splitType,
                                  const float splitSize,
                                  winrt::Windows::Foundation::Size availableSpace) const;

        void ResizeContent(const winrt::Windows::Foundation::Size& newSize);
        void ResizePane(const winrt::Microsoft::Terminal::Settings::Model::ResizeDirection& direction);
        bool NavigateFocus(const winrt::Microsoft::Terminal::Settings::Model::FocusDirection& direction);
        bool SwapPane(const winrt::Microsoft::Terminal::Settings::Model::FocusDirection& direction);
        bool FocusPane(const uint32_t id);

        void UpdateSettings();
        winrt::fire_and_forget UpdateTitle();

        void Shutdown() override;
        void ClosePane();

        void SetTabText(winrt::hstring title);
        winrt::hstring GetTabText() const;
        void ResetTabText();
        void ActivateTabRenamer();

        std::optional<winrt::Windows::UI::Color> GetTabColor();

        void SetRuntimeTabColor(const winrt::Windows::UI::Color& color);
        void ResetRuntimeTabColor();
        void ActivateColorPicker();

        void UpdateZoom(std::shared_ptr<Pane> newFocus);
        void ToggleZoom();
        bool IsZoomed();
        void EnterZoom();
        void ExitZoom();

        std::vector<Microsoft::Terminal::Settings::Model::ActionAndArgs> BuildStartupActions() const;

        int GetLeafPaneCount() const noexcept;

        void TogglePaneReadOnly();
        std::shared_ptr<Pane> GetActivePane() const;
        winrt::TerminalApp::TaskbarState GetCombinedTaskbarState() const;

        std::shared_ptr<Pane> GetRootPane() const { return _rootPane; }

        winrt::TerminalApp::TerminalTabStatus TabStatus()
        {
            return _tabStatus;
        }

        DECLARE_EVENT(ActivePaneChanged, _ActivePaneChangedHandlers, winrt::delegate<>);
        DECLARE_EVENT(ColorSelected, _colorSelected, winrt::delegate<winrt::Windows::UI::Color>);
        DECLARE_EVENT(ColorCleared, _colorCleared, winrt::delegate<>);
        DECLARE_EVENT(TabRaiseVisualBell, _TabRaiseVisualBellHandlers, winrt::delegate<>);
        DECLARE_EVENT(DuplicateRequested, _DuplicateRequestedHandlers, winrt::delegate<>);
        DECLARE_EVENT(SplitTabRequested, _SplitTabRequestedHandlers, winrt::delegate<>);
        DECLARE_EVENT(ExportTabRequested, _ExportTabRequestedHandlers, winrt::delegate<>);
        TYPED_EVENT(TaskbarProgressChanged, IInspectable, IInspectable);

    private:
        std::shared_ptr<Pane> _rootPane{ nullptr };
        std::shared_ptr<Pane> _activePane{ nullptr };
        std::shared_ptr<Pane> _zoomedPane{ nullptr };

        winrt::hstring _lastIconPath{};
        winrt::TerminalApp::ColorPickupFlyout _tabColorPickup{};
        std::optional<winrt::Windows::UI::Color> _themeTabColor{};
        std::optional<winrt::Windows::UI::Color> _runtimeTabColor{};
        winrt::TerminalApp::TabHeaderControl _headerControl{};
        winrt::TerminalApp::TerminalTabStatus _tabStatus{};

        struct ControlEventTokens
        {
            winrt::event_token titleToken;
            winrt::event_token fontToken;
            winrt::event_token colorToken;
            winrt::event_token taskbarToken;
            winrt::event_token readOnlyToken;
            winrt::event_token focusToken;
        };
        std::unordered_map<uint32_t, ControlEventTokens> _controlEvents;

        winrt::event_token _rootClosedToken{};

        std::vector<uint32_t> _mruPanes;
        uint32_t _nextPaneId{ 0 };

        bool _receivedKeyDown{ false };
        bool _iconHidden{ false };

        winrt::hstring _runtimeTabText{};
        bool _inRename{ false };
        winrt::Windows::UI::Xaml::Controls::TextBox::LayoutUpdated_revoker _tabRenameBoxLayoutUpdatedRevoker;

        winrt::TerminalApp::ShortcutActionDispatch _dispatch;

        void _Setup();

        std::optional<Windows::UI::Xaml::DispatcherTimer> _bellIndicatorTimer;
        void _BellIndicatorTimerTick(Windows::Foundation::IInspectable const& sender, Windows::Foundation::IInspectable const& e);

        void _MakeTabViewItem() override;

        winrt::fire_and_forget _UpdateHeaderControlMaxWidth();

        void _CreateContextMenu() override;
        virtual winrt::hstring _CreateToolTipTitle() override;

        void _RefreshVisualState();

        void _DetachEventHandlersFromControl(const uint32_t paneId, const winrt::Microsoft::Terminal::Control::TermControl& control);
        void _AttachEventHandlersToControl(const uint32_t paneId, const winrt::Microsoft::Terminal::Control::TermControl& control);
        void _AttachEventHandlersToPane(std::shared_ptr<Pane> pane);

        void _UpdateActivePane(std::shared_ptr<Pane> pane);

        winrt::hstring _GetActiveTitle() const;

        void _RecalculateAndApplyTabColor();
        void _ApplyTabColor(const winrt::Windows::UI::Color& color);
        void _ClearTabBackgroundColor();

        void _RecalculateAndApplyReadOnly();

        void _UpdateProgressState();

        void _DuplicateTab();

        friend class ::TerminalAppLocalTests::TabTests;
    };
}
