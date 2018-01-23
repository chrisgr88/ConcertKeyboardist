
#pragma once

#include "../DateHeader.h"

//==============================================================================
class AboutWindowComponent    : public Component
{

public:
    AboutWindowComponent()
    {
//        bool showPurchaseButton = false;

        addAndMakeVisible (titleLabel);
        titleLabel.setJustificationType (Justification::centred);
        titleLabel.setFont (Font (35.0f, Font::FontStyleFlags::bold));

        auto buildDate = Time::getCompilationDate();
        addAndMakeVisible (versionLabel);
        versionLabel.setText ("ConcertKeyboardist build: " + String(__CK_SHORT_HASH)
                              + "\nDate: " + String (__CK_BUILD_DATE),
                              dontSendNotification);



        versionLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (copyrightLabel);
        copyrightLabel.setJustificationType (Justification::centred);

//        aboutButton.setJustificationType (Justification::centred);
//        addAndMakeVisible(aboutButton);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromBottom (20);

        auto rightSlice  = bounds.removeFromRight (50);
        auto leftSlice   = bounds.removeFromLeft (50);
        auto centreSlice = bounds;

        //======================================================================
        rightSlice.removeFromRight (20);
//        auto iconSlice = rightSlice.removeFromRight (100);
//        huckleberryLogoBounds = iconSlice.removeFromBottom (100).toFloat();

        //======================================================================
//        juceLogoBounds = leftSlice.removeFromTop (150).toFloat();
//        juceLogoBounds.setWidth (juceLogoBounds.getWidth() + 100);
//        juceLogoBounds.setHeight (juceLogoBounds.getHeight() + 100);

        copyrightLabel.setBounds (centreSlice.removeFromBottom (60));

        //======================================================================
        auto titleHeight = 40;

        centreSlice.removeFromTop ((centreSlice.getHeight() / 2) - (titleHeight / 2));

        titleLabel.setBounds (centreSlice.removeFromTop (titleHeight));

        centreSlice.removeFromTop (10);
        versionLabel.setBounds (centreSlice.removeFromTop (40));

        centreSlice.removeFromTop (10);

//        if (licenseButton.isShowing())
//            licenseButton.setBounds (centreSlice.removeFromTop (25).reduced (25, 0));
//
//        aboutButton.setBounds (centreSlice.removeFromBottom (40));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::bisque);

//        if (juceLogo != nullptr)
//            juceLogo->drawWithin (g, juceLogoBounds.translated (-75, -75), RectanglePlacement::centred, 1.0);

//        if (huckleberryLogo != nullptr)
//            huckleberryLogo->drawWithin (g, huckleberryLogoBounds, RectanglePlacement::centred, 1.0);
    }

private:
    Label titleLabel { "title", "Concert Keyboardist" },
            versionLabel { "version" },
            copyrightLabel { "copyright", String (CharPointer_UTF8 ("\xc2\xa9")) + String (" 2018 Christopher Graham")
                            +String ("\nVST PlugIn Technology by Steinberg Media Technologies")};

//    HyperlinkButton aboutButton { "About Us", URL ("https://github.com/chrisgr88/ConcertKeyboardist/wiki") };
//    HyperlinkButton aboutButton { "About Us", URL ("https://juce.com") };
//    TextButton licenseButton { "Purchase License" };

//    Rectangle<float> juceLogoBounds;

//    ScopedPointer<Drawable> juceLogo { Drawable::createFromImageData (BinaryData::juce_icon_png,
//                                                                      BinaryData::juce_icon_pngSize) };

//    ScopedPointer<Drawable> huckleberryLogo { Drawable::createFromImageData (BinaryData::huckleberry_icon_svg,
//                                                                             BinaryData::huckleberry_icon_svgSize) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutWindowComponent)
};
