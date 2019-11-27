package net.vicp.biggee.jni;

import com.BlankPageDetectDLL;
import com.IdentityCard;
import com.JniDemo;
import com.RemoveBlackBorderDll;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-11-27 08:45
 **/
public class YH_JNI_main {
    public static void initJnilibPath() {
        try {
            addDir("jnilibs");
            System.loadLibrary("opencv_world412");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {


      initJnilibPath();
        File file=new File("C:\\Users\\k1326\\Desktop\\test");
        for (File listFile : file.listFiles()) {
            System.out.println(new BlankPageDetectDLL().BlankPageDetect(listFile.getAbsolutePath()));
        }

        System.out.println("---------------------------");
        IdentityCard.IdentityCardTrans("assert/2.jpg", "assert/out-2.jpg");
        System.out.println(new BlankPageDetectDLL().BlankPageDetect("assert/1.jpg"));
        System.out.println(new BlankPageDetectDLL().BlankPageDetect("assert/2.jpg"));
        System.out.println(new BlankPageDetectDLL().BlankPageDetect("assert/3.jpg"));
        RemoveBlackBorderDll.RemoveBlackBorder("assert/4.jpg", "assert/out-4.jpg");
        JniDemo.ImageRecify("assert/4.jpg", "assert/out-4-1.jpg");  //调整角度
        JniDemo.RemoveUnline("assert/4.jpg", "assert/out-4-2.jpg", 0, 0, 2366, 2638);
   }

    public static void addDir(String s) throws IOException {
        try {
            Field field = ClassLoader.class.getDeclaredField("usr_paths");
            field.setAccessible(true);
            String[] paths = (String[]) field.get(null);
            for (int i = 0; i < paths.length; i++) {
                if (s.equals(paths[i])) {
                    return;
                }
            }
            String[] tmp = new String[paths.length + 1];
            System.arraycopy(paths, 0, tmp, 0, paths.length);
            tmp[paths.length] = s;
            field.set(null, tmp);
        } catch (IllegalAccessException e) {
            throw new IOException("Failed to get permissions to set library path");
        } catch (NoSuchFieldException e) {
            throw new IOException("Failed to get field handle to set library path");
        }
    }
}
