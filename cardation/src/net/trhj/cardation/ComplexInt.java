package net.trhj.cardation;

public class ComplexInt {
    public int x_, y_;

    public ComplexInt(int x, int y) {
        x_ = x;
        y_ = y;
    }

    public String toString() {
        StringBuffer result = new StringBuffer();
        String fmt = new String("%2d: ");
        result.append(String.format(fmt, this.x_));
        for (int i=0; i<this.y_; ++i) {
            result.append("*");
        }
        return result.toString();
    }
}
