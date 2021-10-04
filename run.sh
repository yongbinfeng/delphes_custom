source DelphesEnv.sh

# generate pileup
./DelphesPythia8 cards/converter_card.tcl examples/Pythia8/generatePileUp.cmnd MinBiasMy.root
./root2pileup MinBiasMy.pileup MinBiasMy.root

# generate the root file with pythia
./DelphesPythia8 cards/delphes_card_CMS_My.tcl examples/Pythia8/test_gg.cmnd test_gg.root
