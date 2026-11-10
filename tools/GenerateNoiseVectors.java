import com.bitchat.android.noise.southernstorm.protocol.*;
import java.util.*;
public class GenerateNoiseVectors {
 static byte[] key(int start){byte[] b=new byte[32];for(int i=0;i<32;i++)b[i]=(byte)(start+i);return b;}
 static String hex(byte[] b,int n){StringBuilder s=new StringBuilder();for(int i=0;i<n;i++)s.append(String.format("%02x",b[i]&255));return s.toString();}
 public static void main(String[] args)throws Exception{
  HandshakeState a=new HandshakeState("Noise_XX_25519_ChaChaPoly_SHA256",HandshakeState.INITIATOR);
  HandshakeState b=new HandshakeState("Noise_XX_25519_ChaChaPoly_SHA256",HandshakeState.RESPONDER);
  a.getLocalKeyPair().setPrivateKey(key(1),0);b.getLocalKeyPair().setPrivateKey(key(33),0);
  a.getFixedEphemeralKey().setPrivateKey(key(65),0);b.getFixedEphemeralKey().setPrivateKey(key(97),0);
  a.start();b.start();byte[] m=new byte[512],p=new byte[512];
  int n=a.writeMessage(m,0,null,0,0);System.out.println("m1 "+hex(m,n));b.readMessage(m,0,n,p,0);
  n=b.writeMessage(m,0,null,0,0);System.out.println("m2 "+hex(m,n));a.readMessage(m,0,n,p,0);
  n=a.writeMessage(m,0,null,0,0);System.out.println("m3 "+hex(m,n));b.readMessage(m,0,n,p,0);
  System.out.println("hash "+hex(a.getHandshakeHash(),32));
  CipherStatePair ap=a.split(),bp=b.split();byte[] text="hello".getBytes("UTF-8");
  n=ap.getSender().encryptWithAd(null,text,0,m,0,text.length);System.out.println("txi 00000000"+hex(m,n));
  n=bp.getSender().encryptWithAd(null,text,0,m,0,text.length);System.out.println("txr 00000000"+hex(m,n));
 }
}
