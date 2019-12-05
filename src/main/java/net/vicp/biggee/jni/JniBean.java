package net.vicp.biggee.jni;

import java.util.Arrays;
import java.util.UUID;

/**
 * @program: YH_JNI
 * @description:用于传递对象
 * @author: Biggee
 * @create: 2019-12-05 09:16
 **/
public class JniBean {
    public final String jniUUID = UUID.randomUUID().toString();
    public final long jniTimestamp = System.currentTimeMillis();
    public int jniResultCode = -1;
    public String jniResUltMessage = "";
    public int jniArgCode = -1;
    public String jniArgMessage = "";
    public boolean jniBool = false;
    public double jniDouble = -0.1;
    public Object[] jniObjects;
    public byte[] jniBytes;
    public long[] jniInts;
    public double[] jniDoubles;

    public String getJniUUID() {
        return jniUUID;
    }

    public long getJniTimestamp() {
        return jniTimestamp;
    }

    public int getJniResultCode() {
        return jniResultCode;
    }

    public void setJniResultCode(int jniResultCode) {
        this.jniResultCode = jniResultCode;
    }

    public String getJniResUltMessage() {
        return jniResUltMessage;
    }

    public void setJniResUltMessage(String jniResUltMessage) {
        this.jniResUltMessage = jniResUltMessage;
    }

    public int getJniArgCode() {
        return jniArgCode;
    }

    public void setJniArgCode(int jniArgCode) {
        this.jniArgCode = jniArgCode;
    }

    public String getJniArgMessage() {
        return jniArgMessage;
    }

    public void setJniArgMessage(String jniArgMessage) {
        this.jniArgMessage = jniArgMessage;
    }

    public boolean isJniBool() {
        return jniBool;
    }

    public void setJniBool(boolean jniBool) {
        this.jniBool = jniBool;
    }

    public double getJniDouble() {
        return jniDouble;
    }

    public void setJniDouble(double jniDouble) {
        this.jniDouble = jniDouble;
    }

    public Object[] getJniObjects() {
        return jniObjects;
    }

    public void setJniObjects(Object[] jniObjects) {
        this.jniObjects = jniObjects;
    }

    public byte[] getJniBytes() {
        return jniBytes;
    }

    public void setJniBytes(byte[] jniBytes) {
        this.jniBytes = jniBytes;
    }

    public long[] getJniInts() {
        return jniInts;
    }

    public void setJniInts(long[] jniInts) {
        this.jniInts = jniInts;
    }

    public double[] getJniDoubles() {
        return jniDoubles;
    }

    public void setJniDoubles(double[] jniDoubles) {
        this.jniDoubles = jniDoubles;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof JniBean)) return false;

        JniBean jniBean = (JniBean) o;

        if (getJniTimestamp() != jniBean.getJniTimestamp()) return false;
        if (getJniResultCode() != jniBean.getJniResultCode()) return false;
        if (getJniArgCode() != jniBean.getJniArgCode()) return false;
        if (isJniBool() != jniBean.isJniBool()) return false;
        if (Double.compare(jniBean.getJniDouble(), getJniDouble()) != 0) return false;
        if (getJniUUID() != null ? !getJniUUID().equals(jniBean.getJniUUID()) : jniBean.getJniUUID() != null)
            return false;
        if (getJniResUltMessage() != null ? !getJniResUltMessage().equals(jniBean.getJniResUltMessage()) : jniBean.getJniResUltMessage() != null)
            return false;
        if (getJniArgMessage() != null ? !getJniArgMessage().equals(jniBean.getJniArgMessage()) : jniBean.getJniArgMessage() != null)
            return false;
        // Probably incorrect - comparing Object[] arrays with Arrays.equals
        if (!Arrays.equals(getJniObjects(), jniBean.getJniObjects())) return false;
        if (!Arrays.equals(getJniBytes(), jniBean.getJniBytes())) return false;
        if (!Arrays.equals(getJniInts(), jniBean.getJniInts())) return false;
        return Arrays.equals(getJniDoubles(), jniBean.getJniDoubles());
    }

    @Override
    public int hashCode() {
        int result;
        long temp;
        result = getJniUUID() != null ? getJniUUID().hashCode() : 0;
        result = 31 * result + (int) (getJniTimestamp() ^ (getJniTimestamp() >>> 32));
        result = 31 * result + getJniResultCode();
        result = 31 * result + (getJniResUltMessage() != null ? getJniResUltMessage().hashCode() : 0);
        result = 31 * result + getJniArgCode();
        result = 31 * result + (getJniArgMessage() != null ? getJniArgMessage().hashCode() : 0);
        result = 31 * result + (isJniBool() ? 1 : 0);
        temp = Double.doubleToLongBits(getJniDouble());
        result = 31 * result + (int) (temp ^ (temp >>> 32));
        result = 31 * result + Arrays.hashCode(getJniObjects());
        result = 31 * result + Arrays.hashCode(getJniBytes());
        result = 31 * result + Arrays.hashCode(getJniInts());
        result = 31 * result + Arrays.hashCode(getJniDoubles());
        return result;
    }

    @Override
    public String toString() {
        return "JniBean{" +
                "jniUUID='" + jniUUID + '\'' +
                ", jniTimestamp=" + jniTimestamp +
                ", jniResultCode=" + jniResultCode +
                ", jniResUltMessage='" + jniResUltMessage + '\'' +
                ", jniArgCode=" + jniArgCode +
                ", jniArgMessage='" + jniArgMessage + '\'' +
                ", jniBool=" + jniBool +
                ", jniDouble=" + jniDouble +
                ", jniObjects=" + Arrays.toString(jniObjects) +
                ", jniBytes=" + Arrays.toString(jniBytes) +
                ", jniInts=" + Arrays.toString(jniInts) +
                ", jniDoubles=" + Arrays.toString(jniDoubles) +
                '}';
    }
}
