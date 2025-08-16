# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

FORMS += \
    qt_designer/ViTypeBrowser.ui \
    qt_designer/ViToolExeDialog.ui \
    qt_designer/ViShortcutEditor.ui \
    qt_designer/ViScriptConsoleControl.ui \
    qt_designer/ViScenarioSettings.ui \
    qt_designer/ViReplaceInFilesDialog.ui \
    qt_designer/ViRenameFileDialog.ui \
    qt_designer/ViProjectSettings.ui \
    qt_designer/ViProjectExplorer.ui \
    qt_designer/ViProjectBrowser.ui \
    qt_designer/ViPlatformBrowser.ui \
    qt_designer/ViOptionsDialog.ui \
    qt_designer/ViMainWindow.ui \
    qt_designer/ViHelpAboutDialog.ui \
    qt_designer/ViFindTextDialog.ui \
    qt_designer/ViFindResultsControl.ui \
    qt_designer/ViFindReferencesResultDialog.ui \
    qt_designer/ViFindObject.ui \
    qt_designer/ViFindInFilesDialog.ui \
    qt_designer/ViFileBrowser.ui \
    qt_designer/ViErrorListControl.ui \
    qt_designer/ViEditorForm.ui \
    qt_designer/ViCreateNewFile.ui \
    qt_designer/ViContextDocumentationControl.ui \
    qt_designer/ViBrowserWidget.ui \
    qt_designer/ViWsfOutputControl.ui \
    qt_designer/ViWsfExeDialog.ui \
    qt_designer/ViCommandChainBrowser.ui \
    qt_designer/ViStartupFilesDialog.ui \
    qt_designer/ViScenarioBrowser.ui \
    qt_designer/ViPluginManagerDialog.ui \
    qt_designer/ViEditModeBanner.ui \
    qt_designer/ViPlatPartsBrowser.ui \
    qt_designer/ViPlatPartExpander.ui \
    qt_designer/ViPlatPartDetails.ui \
    qt_designer/ViPpmWidget.ui \
    qt_designer/ViPpmBrowser.ui \
    qt_designer/ViPpmAttribute.ui \
    qt_designer/ViPpmAddPlatformPart.ui \
    qt_designer/ViCreatePlatform.ui \
    qt_designer/ViPpmPosition.ui \
    qt_designer/ViChangeHistory.ui \
    qt_designer/ViMapZoneProperties.ui \
    qt_designer/ViMapWindow.ui \
    qt_designer/ViMapSceneBrowser.ui \
    qt_designer/ViMapRoutePanel.ui \
    qt_designer/ViMapPlatformProperties.ui \
    qt_designer/ViPlotWidget.ui \
    qt_designer/ViMultirunManager.ui \
    qt_designer/ViMultirunSettingsDialog.ui \
    ../source/multirun/ViMultirunSettingsDialog.ui \
    ../source/multirun/ViMultirunManager.ui

RESOURCES += \
    resources/wizard.qrc \
    ../source/resources/wizard.qrc \
    ../source/resources/wizard.qrc

OTHER_FILES += \
    QML/PlatformUI.js \
    QML/PlatformList.js \
    QML/CommandChainUI.js \
    QML/PlatformUI.qml \
    QML/PlatformPartUI.qml \
    QML/PlatformListUI.qml \
    QML/CommandChainUI.qml \
    resources/images/open_arrow_inactive.png \
    resources/images/open_arrow_active.png \
    resources/images/closed_arrow_inactive.png \
    resources/images/closed_arrow_active.png
