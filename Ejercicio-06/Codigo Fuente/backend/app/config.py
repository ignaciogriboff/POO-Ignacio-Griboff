from pydantic_settings import BaseSettings
from functools import lru_cache


class Settings(BaseSettings):
    db_host: str = "173.212.234.190"
    db_port: int = 3306
    db_user: str = "kanban_user"
    db_password: str = ""
    db_name: str = "kanban_db"

    basic_auth_user: str = "admin"
    basic_auth_pass: str = "changeme"

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"

    @property
    def database_url(self) -> str:
        return (
            f"mysql+pymysql://{self.db_user}:{self.db_password}"
            f"@{self.db_host}:{self.db_port}/{self.db_name}"
            f"?charset=utf8mb4"
        )


@lru_cache
def get_settings() -> Settings:
    return Settings()
