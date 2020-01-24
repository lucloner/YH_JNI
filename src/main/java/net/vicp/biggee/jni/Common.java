package net.vicp.biggee.jni;

import org.opencv.core.Mat;
import org.opencv.core.Size;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.imgproc.Imgproc;

import java.io.File;
import java.io.IOException;

/**
 * @program: YH_JNI
 * @description:
 * @author: Biggee
 * @create: 2019-12-02 18:00
 **/
public class Common {
    private static boolean opencv_loaded = false;

    public static void main(String[] args) {
        //test
        enhanceImage(new File("D:\\src\\tiff_CCITT_T.6\\assert\\t\\MYSCAN_20200120_0005.TIF"));
    }

    public static void loadOpenCV() {
        if (!opencv_loaded) {
            try {
                YH_JNI_main.addDir("libs" + File.separator + "windows" + File.separator + "x64");
                System.loadLibrary("opencv_java412");
                opencv_loaded = true;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static File enhanceImage(File img) {
        loadOpenCV();

        final Mat src = Imgcodecs.imread(img.getAbsolutePath());
        Mat tmp1 = new Mat(), tmp2 = new Mat(), dst = new Mat();
        Imgproc.GaussianBlur(src, tmp1, new Size(3, 3), 0);
        Imgproc.cvtColor(tmp1, tmp2, Imgproc.COLOR_BGR2GRAY);
        Imgproc.threshold(tmp2, dst, 254, 255, Imgproc.THRESH_BINARY);
        final File result = new File(img.getParent(), "5_" + img.getName() + "_enhanced.jpg");
        Imgcodecs.imwrite(result.getAbsolutePath(), dst);
        if (result.exists()) {
            return result;
        }
        return null;
    }
}
