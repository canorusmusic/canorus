/* jshint asi:true */

var InstallComponents = [
	"qt.qt5.5130.win64_mingw81"
];

var InstallPath = "C:\\Qt";

var Username = "canorus-qt@protonmail.com"
var Password = "Canorus-qt123"
Password = Password.substring(0,10)

// Public domain via CC0.

function Controller() {
    // It tends to complain about XCode, even if all is okay.
    installer.setMessageBoxAutomaticAnswer("XcodeError", QMessageBox.Ok);

    installer.installationFinished.connect(proceed)
}

function logCurrentPage() {
    var pageName = page().objectName
    var pagePrettyTitle = page().title
    console.log("At page: " + pageName + " ('" + pagePrettyTitle + "')")
}

function page() {
    return gui.currentPageWidget()
}

function proceed(button, delay) {
    gui.clickButton(button || buttons.NextButton, delay)
}

/// Skip welcome page
Controller.prototype.WelcomePageCallback = function() {
    logCurrentPage()
    // For some reason, delay is needed.  Two seconds seems to be enough.
    proceed(buttons.NextButton, 2000)
}

/// Just click next -- that is sign in to Qt account if credentials are
/// remembered from previous installs, or skip sign in otherwise.
Controller.prototype.CredentialsPageCallback = function() {
    logCurrentPage()
    proceed()
}

/// Skip introduction page
Controller.prototype.IntroductionPageCallback = function() {
    logCurrentPage()
    proceed()
}

/// Set target directory
Controller.prototype.TargetDirectoryPageCallback = function() {
    logCurrentPage()
    page().TargetDirectoryLineEdit.text = InstallPath
    proceed()
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    logCurrentPage()
    // Deselect whatever was default, and can be deselected.
    page().deselectAll()

    InstallComponents.forEach(function(component) {
        page().selectComponent(component)
    })

    proceed()
}

/// Agree license
Controller.prototype.LicenseAgreementPageCallback = function() {
    logCurrentPage()
    page().AcceptLicenseRadioButton.checked = true
    gui.clickButton(buttons.NextButton)
}

/// Windows-specific, skip it
Controller.prototype.StartMenuDirectoryPageCallback = function() {
    logCurrentPage()
    gui.clickButton(buttons.NextButton)
}

/// Skip confirmation page
Controller.prototype.ReadyForInstallationPageCallback = function() {
    logCurrentPage()
    proceed()
}

/// Installation in progress, do nothing
Controller.prototype.PerformInstallationPageCallback = function() {
    logCurrentPage()
}

Controller.prototype.FinishedPageCallback = function() {
    logCurrentPage()
    // Deselect "launch QtCreator"
    page().RunItCheckBox.checked = false
    proceed(buttons.FinishButton)
}

/// Question for tracking usage data, refuse it
Controller.prototype.DynamicTelemetryPluginFormCallback = function() {
    logCurrentPage()
    console.log(Object.keys(page().TelemetryPluginForm.statisticGroupBox))
    var radioButtons = page().TelemetryPluginForm.statisticGroupBox
    radioButtons.disableStatisticRadioButton.checked = true
    proceed()
}
