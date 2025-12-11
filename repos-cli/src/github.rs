use crate::auth::get_token;
use anyhow::Result;
use octocrab::Octocrab;

pub async fn list_repos() -> Result<()> {
    let token = get_token()?;
    let octocrab = Octocrab::builder()
        .personal_token(token)
        .build()?;

    let user = octocrab.current().user().await?;
    println!("Fetching repositories for {}...", user.login);

    let mut page = octocrab
        .current()
        .list_repos_for_authenticated_user()
        .per_page(100)
        .send()
        .await?;

    let mut repos = page.take_items();

    while let Some(mut next_page) = octocrab.get_page::<octocrab::models::Repository>(&page.next).await? {
        repos.extend(next_page.take_items());
        page = next_page;
    }

    for repo in repos {
        println!("{}", repo.name);
    }

    Ok(())
}
