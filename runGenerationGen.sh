#!/bin/bash

source DelphesEnv.sh

./DelphesPythia8 cards/converter_card.tcl examples/Pythia8/testPileUp.cmnd MinBiasMy_5K.root

./root2pileup MinBiasMy_5K.pileup MinBiasMy_5K.root 

./DelphesPythia8 cards/delphes_card_CMS_My_SSLPUPPI.tcl examples/Pythia8/testTTbar.cmnd testTTbar.root
