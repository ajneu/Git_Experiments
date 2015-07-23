// Example code: https://libgit2.github.com/libgit2/ex/HEAD/general.html
// http://ben.straub.cc/2013/04/02/libgit2-checkout/
// Test libgit2: build the following structure: http://git-scm.com/book/en/v2/Git-Branching-Branching-Workflows#Topic-Branches

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp> /* <experimental/filesystem> is not yet common everywhere */

#include "git2.h"

namespace fs = boost::filesystem;


#define CHECK_ERROR( error)         ((error < 0) ? check_error(error, __FILE__, __LINE__) : (void)0)
#define CHECK_ERROR2(error, action) ((error < 0) ? check_error(error, __FILE__, __LINE__, action) : (void)0)

void check_error(int error_code, const char *file, int line, const char *action = "")
{
  if (!error_code)
    return;

  const git_error *error = giterr_last();

  printf("\n"
         "Error in\n"
         "file %s\n"
         "line %d\n"
         "Error-Detail: %d/%d %s - %s\n", file, line, error_code, error->klass, action,
         (error && error->message) ? error->message : "???");
  exit(error_code);
}

void append_to_file(const std::string& filepath)
{
  std::ofstream os{filepath, std::fstream::app};
  if (!os) {
    std::cerr << "Cannot open " << filepath << " for writing" << std::endl;
    exit(-1);
  }
  os << "a" << std::endl;
}

git_commit * get_last_commit ( git_repository * repo ) // must call git_commit_free() on the value returned
{
#ifdef V2
  // http://stackoverflow.com/a/15717626
  int rc;
  git_commit * commit = NULL; /* the result */
  git_oid oid_parent_commit;  /* the SHA1 for last commit */
  
  /* resolve HEAD into a SHA1 */
  rc = git_reference_name_to_id( &oid_parent_commit, repo, "HEAD" );
  if ( rc == 0 ) {
    /* get the actual commit structure */
    rc = git_commit_lookup( &commit, repo, &oid_parent_commit );
    if ( rc == 0 ) {
      return commit;
    }
  }
  return NULL;
#else
  git_commit * commit = NULL;
  return ((git_revparse_single( (git_object**) &commit, repo, "HEAD" ) == 0)
          ? commit
          : NULL);
#endif
}




git_commit * get_nth_ancestor_commit ( git_repository * repo, unsigned int n ) // must call git_commit_free() on the value returned
{
  //int git_commit_nth_gen_ancestor(git_commit **ancestor, const git_commit *commit, unsigned int n);             
  // adaption of http://stackoverflow.com/a/15717626
  int rc;
  git_commit * lastCommit       = NULL;
  git_commit * ancestorCommit   = NULL; /* the result */

  git_oid oid_parent_commit;  /* the SHA1 for last commit */

  /* resolve HEAD into a SHA1 */
  rc = git_reference_name_to_id( &oid_parent_commit, repo, "HEAD" );
  if ( rc == 0 ) {
    /* get the actual commit structure */
    rc = git_commit_lookup( &lastCommit, repo, &oid_parent_commit );
    if ( rc == 0 ) {
      rc = git_commit_nth_gen_ancestor(&ancestorCommit, lastCommit, n);
      git_commit_free(lastCommit);
      if ( rc == 0 ) {
        return ancestorCommit;
      }
    }
  }
  return NULL;
}




static void create_file_commit(git_repository *repo, const std::string &filepath /* relative to working directory */, const std::string &commit_msg)
{
  git_signature *sig   = NULL;
  git_index *index     = NULL;
  git_tree *tree       = NULL;
  git_commit* old_head = NULL;
  git_oid tree_id, commit_id;

  int error;

  if (git_signature_default(&sig, repo) < 0) {
    /* Unable to create a commit signature. Perhaps 'user.name' and 'user.email' are not set. Use a default */
    error = git_signature_now(&sig, "default_name", "default_name@example.com");
    CHECK_ERROR(error);
  }

  error = git_repository_index(&index, repo);
  CHECK_ERROR(error); // if (error < 0): Could not open repository index

  error = git_index_read(index, 0);     // not necessary here since it is preceded by git_repository_index
  CHECK_ERROR(error); // if (error < 0): Could not read index
  
  error = git_index_add_bypath(index, filepath.c_str());
  CHECK_ERROR(error); // if (error < 0): Could not add file to repository index

  error = git_index_write(index);
  CHECK_ERROR(error); // if (error < 0): Unable to write initial tree from index
  
  error = git_index_write_tree(&tree_id, index);
  CHECK_ERROR(error); // if (error < 0): Unable to write initial tree from index

  git_index_free(index);

  error = git_tree_lookup(&tree, repo, &tree_id);
  CHECK_ERROR(error); // if (error < 0): Could not look up initial tree
  
  if (git_repository_head_unborn(repo)) { // if ((old_head = get_last_commit(repo)), old_head == NULL) {
    //std::cout << "old_head NULL" << std::endl;
    error = git_commit_create_v(&commit_id, repo, "HEAD", sig, sig,
                                NULL, commit_msg.c_str(), tree, 0);
  } else {
    old_head = get_last_commit(repo); // must call git_commit_free(old_head) afterwards

    //std::cout << "old_head is not NULL" << std::endl;
    error = git_commit_create_v(&commit_id, repo, "HEAD", sig, sig,
                                NULL, commit_msg.c_str(), tree, 1, old_head);
  }
  CHECK_ERROR(error); // if (error < 0): Could not create the commit

  git_commit_free(old_head);
  git_tree_free(tree);
  git_index_free(index);
  git_signature_free(sig);
}







int main()
{
  const std::string work_path = "proj";
  const std::string filename  = "a.txt";
  const std::string filepath  = work_path + '/' + filename;
  
  if (fs::exists(work_path)) {
    std::cout << "remove " << work_path << std::endl;
    fs::remove_all(work_path);
  }

  {
    git_libgit2_init();
    {
      git_repository *repo = NULL;
      {
        std::cout << "initializing git working dir in dir: " << work_path << std::endl;
        int error = git_repository_init(&repo, work_path.c_str(), false /* false : not bare */);
        CHECK_ERROR(error);

        append_to_file(filepath.c_str());
        create_file_commit(repo, filename, "C0");

        append_to_file(filepath.c_str());
        create_file_commit(repo, filename, "C1");

        git_reference *ref_branch = NULL;
        {
          git_commit *last_commit = get_last_commit(repo);
          error = git_branch_create(&ref_branch, repo, "iss91", last_commit, 0);
          CHECK_ERROR(error);
          git_commit_free(last_commit);

          //error = git_checkout_head(repo, NULL);
          //CHECK_ERROR(error);
          
          error = git_repository_set_head(repo, "refs/heads/iss91");
          CHECK_ERROR(error);

          append_to_file(filepath.c_str());
          create_file_commit(repo, filename, "C2");

          error = git_repository_set_head(repo, "refs/heads/master");
          CHECK_ERROR(error);

          append_to_file(filepath.c_str());
          create_file_commit(repo, filename, "C3");
          
          error = git_repository_set_head(repo, "refs/heads/iss91");
          CHECK_ERROR(error);

          append_to_file(filepath.c_str());
          create_file_commit(repo, filename, "C4");

          append_to_file(filepath.c_str());
          create_file_commit(repo, filename, "C5");

          append_to_file(filepath.c_str());
          create_file_commit(repo, filename, "C6");


          {
            git_commit *ancestorCommit = NULL;
            ancestorCommit = get_nth_ancestor_commit (repo, 2U); // must call git_commit_free() on the value returned
            if (ancestorCommit != NULL) {
              git_reference *ref_branch = NULL;
              {
                error = git_branch_create(&ref_branch, repo, "iss91v2", ancestorCommit, 0);
                CHECK_ERROR(error);
                git_commit_free(ancestorCommit);
                
                //error = git_checkout_head(repo, NULL);
                //CHECK_ERROR(error);
                
                error = git_repository_set_head(repo, "refs/heads/iss91v2");
                CHECK_ERROR(error);

                append_to_file(filepath.c_str());
                create_file_commit(repo, filename, "C7");

                append_to_file(filepath.c_str());
                create_file_commit(repo, filename, "C8");

                error = git_repository_set_head(repo, "refs/heads/master");
                CHECK_ERROR(error);

                append_to_file(filepath.c_str());
                create_file_commit(repo, filename, "C9");

                append_to_file(filepath.c_str());
                create_file_commit(repo, filename, "C10");

                error = git_repository_set_head(repo, "refs/heads/iss91v2");
                CHECK_ERROR(error);

                append_to_file(filepath.c_str());
                create_file_commit(repo, filename, "C11");

                error = git_repository_set_head(repo, "refs/heads/master");
                CHECK_ERROR(error);



                git_reference *ref_branch2 = NULL;
                {
                  git_commit *last_commit = get_last_commit(repo);
                  error = git_branch_create(&ref_branch, repo, "dumbidea", last_commit, 0);
                  CHECK_ERROR(error);
                  git_commit_free(last_commit);
                  
                  //error = git_checkout_head(repo, NULL);
                  //CHECK_ERROR(error);
                  
                  error = git_repository_set_head(repo, "refs/heads/dumbidea");
                  CHECK_ERROR(error);

                  
                  append_to_file(filepath.c_str());
                  create_file_commit(repo, filename, "C12");

                  append_to_file(filepath.c_str());
                  create_file_commit(repo, filename, "C13");

                }
                git_reference_free(ref_branch2);
              }
            }
          }
        }
        git_reference_free(ref_branch);
      }
      git_repository_free(repo);
    }
    git_libgit2_shutdown();
  }
  return 0;
}
