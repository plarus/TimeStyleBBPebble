#!/bin/bash

char_arr=(0 1 2 3 4 5 6 7 8 9)

for i in ${char_arr[@]}; do
  fctx-compiler ../other/avenir-next-regular.svg -r [$i]
  mv AvenirNextLTW01RegularRegular.ffont ../resources/fonts/digit_$i.ffont

  fctx-compiler ../other/avenirnext-demibold.svg -r [$i]
  mv AvenirNextDemiBold.ffont ../resources/fonts/digit_bold_$i.ffont

  fctx-compiler ../other/LECO_1976-Regular.1.svg -r [$i]
  mv LECO1976-Regular.ffont ../resources/fonts/digit_leco_$i.ffont
done

fctx-compiler ../other/avenir-next-regular.svg -r [:]
mv AvenirNextLTW01RegularRegular.ffont ../resources/fonts/digit_colon.ffont

fctx-compiler ../other/avenirnext-demibold.svg -r [:]
mv AvenirNextDemiBold.ffont ../resources/fonts/digit_bold_colon.ffont

fctx-compiler ../other/LECO_1976-Regular.1.svg -r [:]
mv LECO1976-Regular.ffont ../resources/fonts/digit_leco_colon.ffont

fctx-compiler ../other/avenir-next-regular.svg -r [\ ]
mv AvenirNextLTW01RegularRegular.ffont ../resources/fonts/digit_space.ffont

fctx-compiler ../other/avenirnext-demibold.svg -r [\ ]
mv AvenirNextDemiBold.ffont ../resources/fonts/digit_bold_space.ffont

fctx-compiler ../other/LECO_1976-Regular.1.svg -r [\ ]
mv LECO1976-Regular.ffont ../resources/fonts/digit_leco_space.ffont
