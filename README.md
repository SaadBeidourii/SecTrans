# SecTrans for MacroHard

# How to run ?
``` bash
mkdir obj
mkdir bin
```
## run server:
``` bash
docker build -t sectrans .
docker run -p 4000:3000 -it sectrans
```
> hint, you may need sudo depending on your system and your rights

## run client:
```bash
make client
./bin/client -up {path to your file}
```

# how is this supposed to work and what makes it secure ?
## AUTHENTICATION !!1!1
for you to be able to do anything on the client app (i.e upload a file, download a file, or even just see that the files in our prestige server are) you need to be authenticated, meaning any time you run secTrans you need to provide a username and password.
## Hashing
when you upload a file the client app will also upload a hash of that file. meaning that whenever it is downloaded it can be compared next to it's hash to see if it's the file we want or not.
## Encryption!!!
we will use some sort of asymetric public/private encryption that is end to end to ensure the sec in trans (unlike whatsapp)
> we will use the RSA algorithm for encrypting and signing the file. it will be sent block by block to the server encrypted using  it's public key, so the server will decrypt each block using it's private key and store it. the first and last blocks sent have to be identifiers that what we're sending is a new file, the name of the file. the last block should contain information signifying that we have ended the transfer, then finally send the signature of the file.
### Pay attention! Development will be done on separate branches and merged into develop. then when all works as we want it to work, we can merge it into main.
