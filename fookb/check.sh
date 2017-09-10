rm /tmp/fookb*

(make superpuperclean
autoconf &&
./configure &&
make &&
mv fookb /tmp/fookb) &&

(make superpuperclean &&
autoconf &&
./configure --enable-wmaker &&
make &&
mv fookb /tmp/fookb.ewm) &&

(make superpuperclean &&
autoconf &&
./configure --disable-wmaker &&
make &&
mv fookb /tmp/fookb.dwm) &&

(make superpuperclean &&
autoconf &&
./configure --enable-libWUtil &&
make &&
mv fookb /tmp/fookb.ewu) &&

(make superpuperclean &&
autoconf &&
./configure --disable-libWUtil &&
make &&
mv fookb /tmp/fookb.dwu) &&

(make superpuperclean &&
autoconf &&
./configure --enable-wmaker --enable-libWUtil &&
make &&
mv fookb /tmp/fookb.ewm.ewu) &&

(make superpuperclean &&
autoconf &&
./configure --enable-wmaker --disable-libWUtil &&
make &&
mv fookb /tmp/fookb.ewm.dwu) &&

(make superpuperclean &&
autoconf &&
./configure --disable-wmaker --enable-libWUtil &&
make &&
mv fookb /tmp/fookb.dwm.ewu) &&

(make superpuperclean &&
autoconf &&
./configure --disable-wmaker --disable-libWUtil &&
make &&
mv fookb /tmp/fookb.dwm.dwu) &&

make clean

