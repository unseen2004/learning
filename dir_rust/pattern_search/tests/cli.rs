use assert_cmd::prelude::*;
use assert_fs::prelude::*;
use predicates::prelude::*;
use assert_cmd::Command;

#[test]
fn finds_lines_in_file() {
    let file = assert_fs::NamedTempFile::new("sample.txt").unwrap();
    file.write_str("Alpha\nBeta\nalphabet\n").unwrap();

    let mut cmd = Command::cargo_bin("rgrep").unwrap();
    cmd.arg("Alpha").arg(file.path());
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Alpha").and(predicate::str::contains("alphabet")));
}

#[test]
fn ignore_case_and_count() {
    let file = assert_fs::NamedTempFile::new("data.txt").unwrap();
    file.write_str("Rust\nrusty\nRUST\ntrust\n").unwrap();

    let mut cmd = Command::cargo_bin("rgrep").unwrap();
    cmd.args(["-i", "-c", "rust"]).arg(file.path());
    cmd.assert().success().stdout("3\n");
}

#[test]
fn stdin_support() {
    Command::cargo_bin("rgrep").unwrap()
        .arg("foo")
        .write_stdin("bar\nfoo\nbaz\n")
        .assert()
        .success()
        .stdout("foo\n");
}