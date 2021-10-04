source DelphesEnv.sh

# generate 1M pileup
./DelphesPythia8 cards/converter_card.tcl examples/Pythia8/generatePileUp.cmnd MinBiasMy_1M.root

# generate the root file with pythia
./DelphesPythia8 cards/delphes_card_CMS_My_SSLPUPPI.tcl examples/Pythia8/test_Znunu.cmnd test_Znunu.root
