

import java.io.File
import scala.io.Source

object Main extends App {

  def readMatrix(file: File): Array[Array[Double]] = {
    val length = Source.fromFile(file).getLines().size
    val result: Array[Array[Double]] = new Array(length)
    val arr: Array[Double] = Source.fromFile(file).getLines().toArray.flatMap(_.split(","))map(_.toDouble)
    var i: Int = 0;
    var j: Int = 0;
    println(length)
    while (i < length) {
      j = 0;
      val temp: Array[Double] = new Array(length)
      while (j < length) {
        temp.update(j, arr(i * length + j))
        j+=1;
      }
      result.update(i, temp)
      i+=1;
    }
    result
  }

  def multiply(m1: Array[Array[Double]], m2: Array[Array[Double]]): Array[Array[Double]] = {
    val res = Array.ofDim[Double](m1.length, m2(0).length)
    val M1_COLS = m1(0).length
    val M1_ROWS = m1.length
    val M2_COLS = m2(0).length

    @inline def singleThreadedMultiplicationFAST(start_row: Int, end_row: Int) {
      var col, i = 0
      var sum = 0.0
      var row = start_row

      // while statements are much faster than for statements
      while (row < end_row) {
        col = 0
        while (col < M2_COLS) {
          i = 0; sum = 0
          while (i < M1_COLS) {
            sum += m1(row)(i) * m2(i)(col)
            i += 1
          }

          res(row)(col) = sum
          col += 1

        }; row += 1
      }
    }

    (0 until M1_ROWS).par.foreach(i =>
      singleThreadedMultiplicationFAST(i, i + 1))

    res
  }

  override def main(args: Array[String]) {
	  val m1: Array[Array[Double]] = readMatrix(new File("m1.txt"))
	  val m2: Array[Array[Double]] = readMatrix(new File("m2.txt"))
	  println("Start multiply")
	  val start: Long = System.currentTimeMillis()
	  val res = multiply(m1, m2)
	  println("Time = "+(System.currentTimeMillis()-start))
  }
}