#include <Eigen/Core>
#include <boost/python.hpp>
#include <numpy/arrayobject.h>

namespace boopy
{
  namespace bp = boost::python;

  template <typename SCALAR>  struct NumpyEquivalentType {};
  template <> struct NumpyEquivalentType<double>  { enum { type_code = NPY_DOUBLE };};
  template <> struct NumpyEquivalentType<int>     { enum { type_code = NPY_INT    };};
  template <> struct NumpyEquivalentType<float>   { enum { type_code = NPY_FLOAT  };};

  /* --- TO PYTHON -------------------------------------------------------------- */
  template< typename MatType >
  struct EigenMatrix_to_python_matrix
  {
    static PyObject* convert(MatType const& mat)
    {
      typedef typename MatType::Scalar T;
      const int R  = mat.rows(), C = mat.cols();

      npy_intp shape[2] = { R,C };
      PyArrayObject* pyArray = (PyArrayObject*)
	PyArray_SimpleNew(2, shape,
			  NumpyEquivalentType<T>::type_code);

      T* pyData = (T*)PyArray_DATA(pyArray);
      Eigen::Map< Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> > pyMatrix(pyData,R,C);
      pyMatrix = mat;

      return (PyObject*)pyArray;
    }
  };
  
  /* --- FROM PYTHON ------------------------------------------------------------ */
  struct EigenMatrix_from_python_array
  {

    EigenMatrix_from_python_array()
    {
      bp::converter::registry
	::push_back(&convertible,
		    &construct,
		    bp::type_id<Eigen::MatrixXd>());
    }
 
    // Determine if obj_ptr can be converted in a Eigenvec
    static void* convertible(PyObject* obj_ptr)
    {
      typedef Eigen::MatrixXd MatType;
      typedef MatType::Scalar T;

      if (!PyArray_Check(obj_ptr)) return 0;

      if (PyArray_NDIM(obj_ptr) != 2)
	if ( (PyArray_NDIM(obj_ptr) !=1) || (MatType::IsVectorAtCompileTime) )
	  return 0;

      if (PyArray_ObjectType(obj_ptr, 0) != NumpyEquivalentType<T>::type_code)
	return 0;
      
      if (!(PyArray_FLAGS(obj_ptr) & NPY_ALIGNED))
	{
	  std::cerr << "NPY non-aligned matrices are not implemented." << std::endl;
	  return 0;
	}
      
      return obj_ptr;
    }
 
    // Convert obj_ptr into a Eigenvec
    static void construct(PyObject* pyObj,
			  bp::converter::rvalue_from_python_stage1_data* memory)
    {
      typedef Eigen::MatrixXd MatType;
      typedef MatType::Scalar T;
      using namespace Eigen;

      PyArrayObject * pyArray = reinterpret_cast<PyArrayObject*>(pyObj);
      int ndims = PyArray_NDIM(pyArray);
      assert(ndims == 2); // TODO: handle vectors

      int itemsize = PyArray_ITEMSIZE(pyArray);
      int stride1 = PyArray_STRIDE(pyArray, 0) / itemsize;
      int stride2 = PyArray_STRIDE(pyArray, 1) / itemsize;
      std::cout << "STRIDE = " << stride1 << " x " << stride2 << std::endl;

      int R = MatrixXd::RowsAtCompileTime;
      int C = MatrixXd::ColsAtCompileTime;
      if (R == Eigen::Dynamic) R = PyArray_DIMS(pyArray)[0];
      else	               assert(PyArray_DIMS(pyArray)[0]==R);

      if (C == Eigen::Dynamic) C = PyArray_DIMS(pyArray)[1];
      else	               assert(PyArray_DIMS(pyArray)[1]==C);

      T* pyData = reinterpret_cast<T*>(PyArray_DATA(pyArray));

      void* storage = ((bp::converter::rvalue_from_python_storage<MatrixXd>*)
		       (memory))->storage.bytes;
      MatrixXd & mat = * new (storage) MatrixXd(R,C);
      for(int i=0;i<R;++i) 
	for(int j=0;j<C;++j) 
	  mat(i,j) = pyData[i*C+j];

      memory->convertible = storage;
    }
  };


}

Eigen::MatrixXd test()
{
  Eigen::MatrixXd mat = Eigen::MatrixXd::Random(3,6);
  std::cout << "EigenMAt = " << mat << std::endl;
  return mat;
}
Eigen::VectorXd testVec()
{
  Eigen::VectorXd mat = Eigen::VectorXd::Random(6);
  std::cout << "EigenVec = " << mat << std::endl;
  return mat;
}

void test2( Eigen::MatrixXd mat )
{
  std::cout << mat << std::endl;
}

BOOST_PYTHON_MODULE(libeigentemplate)
{
  import_array();
  namespace bp = boost::python;
  bp::to_python_converter<Eigen::MatrixXd,
			  boopy::EigenMatrix_to_python_matrix<Eigen::MatrixXd> >();
  bp::to_python_converter<Eigen::VectorXd,
			  boopy::EigenMatrix_to_python_matrix<Eigen::VectorXd> >();
  boopy::EigenMatrix_from_python_array();

  bp::def("test", test);
  bp::def("testVec", testVec);
  bp::def("test2", test2);
}
