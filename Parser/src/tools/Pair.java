/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package tools;

/**
 * 
 * @author Andrej
 * 
 * @param <L> type of left parameter
 * @param <R> type of right parameter
 */
public class Pair<L,R> {

    /**
     * Left parameter
     */
    public final L left;

    /**
     * Right parameter
     */
    public final R right;

    /**
     * Creates new Pair
     * 
     * @param left left parameter
     * @param right right parameter
     */
    public Pair(L left, R right) {
        this.left = left;
        this.right = right;
    }

    /**
     *
     * @param <L> type of left parameter
     * @param <R> type of right parameter
     * @param left left parameter
     * @param right right parameter
     * @return new pair
     */
    public static <L,R> Pair<L,R> of(L left, R right){
        return new Pair<>(left, right);
    }
}