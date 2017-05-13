#!/usr/bin/perl

# if a CR is needed for only one (or a subset of all) applications, just edit the following list and remove the pool names that are not necessary. THESE MUST MATCH COLUMN D of INVPROD

@prodApps = (
             "Historical",
             "TAX",
             "ShopIS",
             "ShopIS200",
             "ShopISLMC",
             "PricingV2",
             "ShopISTVCY",
             "ShopMIP",
             "FD",
             "ShopMIP200",
             "ShopMIPLMC",
             "ShopMIPTVCY",
             "ESVII",
             "ShopHist",
	     "ShopMIPIBFM",
             "ShopISIBFM",
             "ShopISGEOI",
             "ShopMIPGEOI"
            );

@certApps = (
             "ShopMIP",
             "Historical",
             "TAX",
             "ShopIS",
             "PricingV2",
             "FD",
             "ESVII",
             "ShopHist"
            );

# no need to change the following lists if the above is changed. KEYS MUST MATCH the above list.
%deployList = (
               "Historical", "historical",
               "TAX", "tax",
               "ShopIS", "shoppingis",
               "ShopIS200", "shoppingis",
               "ShopISLMC", "shoppingis",
               "PricingV2", "pricingv2",
               "ShopISTVCY", "shoppingis",
               "ShopMIP", "shopping",
               "FD", "faredisplay",
               "ShopMIP200", "shopping",
               "ShopMIPLMC", "shopping",
               "ShopMIPTVCY", "shopping",
               "ShopHist", "shoppinghist",
               "ESVII", "shoppingesv",
	       "ShopMIPIBFM", "shopping",
	       "ShopISIBFM", "shoppingis",
	       "ShopISGEOI", "shoppingis",
               "ShopMIPGEOI", "shopping"
              );

%friendlyPoolName = (
                     "TAX", "Tax",
                     "FD", "FareDisplay",
                     "PricingV2", "Pricing",
                     "Historical", "Historical Pricing",
                     "ShopIS", "General ShoppingIS",
                     "ShopIS200", "C-ShoppingIS",
                     "ShopISLMC", "D-ShoppingIS",
                     "ShopISTVCY", "E-ShoppingIS",
                     "ShopMIP", "General ShoppingMIP",
                     "ShopMIP200", "C-ShoppingMIP",
                     "ShopMIPLMC", "D-ShoppingMIP",
                     "ShopMIPTVCY", "E-ShoppingMIP",
                     "ESVII", "ShoppingESV",
                     "ShopHist", "Historical Shopping",
                     "ShopMIPIBFM", "G-ShoppingMIP",
                     "ShopISIBFM", "G-ShoppingIS",
                     "ShopISGEOI", "H-ShoppingIS",
                     "ShopMIPGEOI", "H-ShoppingMIP"
                    );

%shoppingPoolName = (
                     "ShopIS", "A",
                     "ShopIS200", "C",
                     "ShopISLMC", "D",
                     "ShopISTVCY", "E",
                     "ShopISIBFM", "G",
                     "ShopISGEOI", "H",
                     "ShopMIP", "A",
                     "ShopMIP200", "C",
                     "ShopMIPLMC", "D",
                     "ShopMIPTVCY", "E",
                     "ShopMIPIBFM", "G",
                     "ShopMIPGEOI", "H"
                     );

